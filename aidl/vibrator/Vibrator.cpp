#define LOG_TAG "android.hardware.vibrator-service.msm8916"
#include <android-base/logging.h>

#include <cutils/properties.h>
#include <cinttypes>
#include <cmath>
#include <iostream>
#include <fstream>
#include <thread>

#include "Vibrator.h"

namespace aidl {
namespace android {
namespace hardware {
namespace vibrator {

static const char *VIBRATOR_AMPLITUDE_LIGTH_PROP = "ro.vibrator.hal.amplitude.light";
static const char *VIBRATOR_AMPLITUDE_MEDIUM_PROP = "ro.vibrator.hal.amplitude.medium";
static const char *VIBRATOR_AMPLITUDE_STRONG_PROP = "ro.vibrator.hal.amplitude.strong";

static const char *VIBRATOR_EFFECT_CLICK_DURATION_PROP = "ro.vibrator.hal.click.duration";
static const char *VIBRATOR_EFFECT_TICK_DURATION_PROP = "ro.vibrator.hal.tick.duration";

Vibrator::Vibrator(std::ofstream&& enable) : mEnable(std::move(enable)) {
    mVtgLevel.open(VIBRATOR_LEVEL_PATH);

    mVtgDefault = 25;
    mVtgMax = 31;
    mVtgMin = 18;

    mEffectClickDuration = (int32_t)property_get_int32(VIBRATOR_EFFECT_CLICK_DURATION_PROP, 40);
    mEffectTickDuration = (int32_t)property_get_int32(VIBRATOR_EFFECT_TICK_DURATION_PROP, 20);

    mAmplitudeLight = (uint8_t)property_get_int32(VIBRATOR_AMPLITUDE_LIGTH_PROP, 45);
    mAmplitudeMedium = (uint8_t)property_get_int32(VIBRATOR_AMPLITUDE_MEDIUM_PROP, 65);
    mAmplitudeStrong = (uint8_t)property_get_int32(VIBRATOR_AMPLITUDE_STRONG_PROP, 85);
}

ndk::ScopedAStatus Vibrator::getCapabilities(int32_t* _aidl_return) {
    *_aidl_return = IVibrator::CAP_ON_CALLBACK | IVibrator::CAP_PERFORM_CALLBACK;
    if (mVtgLevel.is_open()) {
        *_aidl_return |= IVibrator::CAP_AMPLITUDE_CONTROL;
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::off() {
    mEnable << 0 << std::endl;
    if (!mEnable) {
        LOG(ERROR) << "Failed to turn vibrator off. Error: " << errno << " - " << strerror(errno);
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_SERVICE_SPECIFIC));
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::on(int32_t timeoutMs,
                                const std::shared_ptr<IVibratorCallback>& callback) {
    mEnable << timeoutMs << std::endl;
    if (!mEnable) {
        LOG(ERROR) << "Failed to turn vibrator on. Error: " << errno << " - " << strerror(errno);
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_SERVICE_SPECIFIC));
    }

    if (callback != nullptr) {
        std::thread([=] {
            usleep(timeoutMs * 1000);
            if (!callback->onComplete().isOk()) {
                LOG(ERROR) << "Failed to call onComplete";
            }
        }).detach();
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::perform(Effect effect, EffectStrength es, const std::shared_ptr<IVibratorCallback>& callback, int32_t* _aidl_return) {
    float amplitude = 25.0f;
    switch (es) {
        case EffectStrength::LIGHT:
            amplitude = 18.0f;
            break;
        case EffectStrength::MEDIUM:
            amplitude = 25.0f;
            break;
        case EffectStrength::STRONG:
        default:
            amplitude = 31.0f;
            break;
    }

    ndk::ScopedAStatus status = ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
    switch (effect) {
        case Effect::CLICK:
            setAmplitude(amplitude);
            *_aidl_return = mEffectClickDuration;
            status = on(mEffectClickDuration, callback);
            break;
        case Effect::TICK:
            setAmplitude(amplitude);
            *_aidl_return = mEffectTickDuration;
            status = on(mEffectTickDuration, callback);
            break;
        default:
            break;
    }

    return status;
}

ndk::ScopedAStatus Vibrator::getSupportedEffects(std::vector<Effect>* _aidl_return) {
    *_aidl_return = {Effect::CLICK, Effect::TICK};
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::setAmplitude(float amplitude) {
    if (!mVtgLevel.is_open()) {
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
    }

    if (amplitude < 12.0f || amplitude > 31.0f) {
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_ILLEGAL_ARGUMENT));
    }

    long intensity = (long)amplitude;
    mVtgLevel << intensity << std::endl;
    if (!mVtgLevel) {
        LOG(ERROR) << "Failed to set amplitude. Error: " << errno << " - " << strerror(errno);
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_SERVICE_SPECIFIC));
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::setExternalControl(bool enabled __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getCompositionDelayMax(int32_t* maxDelayMs __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getCompositionSizeMax(int32_t* maxSize __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getSupportedPrimitives(std::vector<CompositePrimitive>* supported __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getPrimitiveDuration(CompositePrimitive primitive __unused,
                                                  int32_t* durationMs __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::compose(const std::vector<CompositeEffect>& composite __unused,
                                     const std::shared_ptr<IVibratorCallback>& callback __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getSupportedAlwaysOnEffects(std::vector<Effect>* _aidl_return __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::alwaysOnEnable(int32_t id __unused, Effect effect __unused,
                                            EffectStrength strength __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::alwaysOnDisable(int32_t id __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

}  // namespace vibrator
}  // namespace hardware
}  // namespace android
}  // namespace aidl
