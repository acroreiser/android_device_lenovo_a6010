/*
 * Copyright (C) 2017-2020 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "VibratorService"

#include <log/log.h>

#include <hardware/hardware.h>
#include <hardware/vibrator.h>

#include "Vibrator.h"

#include <cinttypes>
#include <cmath>
#include <iostream>
#include <fstream>


namespace android {
namespace hardware {
namespace vibrator {
namespace V1_1 {
namespace implementation {

static constexpr uint32_t CLICK_TIMING_MS = 40;
static constexpr uint32_t TICK_TIMING_MS = 20;

using Status = ::android::hardware::vibrator::V1_0::Status;
using EffectStrength = ::android::hardware::vibrator::V1_0::EffectStrength;

Vibrator::Vibrator(std::ofstream&& enable, std::ofstream&& amplitude) :
        mEnable(std::move(enable)),
        mAmplitude(std::move(amplitude)) {}

// Methods from ::android::hardware::vibrator::V1_1::IVibrator follow.
Return<Status> Vibrator::on(uint32_t timeout_ms) {
    mEnable << timeout_ms << std::endl;
    if (!mEnable) {
        ALOGE("Failed to turn vibrator on (%d): %s", errno, strerror(errno));
        return Status::UNKNOWN_ERROR;
    }
    return Status::OK;
}

Return<Status> Vibrator::off()  {
    mEnable << 0 << std::endl;
    if (!mEnable) {
        ALOGE("Failed to turn vibrator off (%d): %s", errno, strerror(errno));
        return Status::UNKNOWN_ERROR;
    }
    return Status::OK;
}

Return<bool> Vibrator::supportsAmplitudeControl()  {
    return true;
}

Return<Status> Vibrator::setAmplitude(uint8_t amplitude) {
	long buf;

    if(amplitude < 12)
        amplitude = 12;

    if(amplitude > 31)
        amplitude = 31;

    buf = amplitude;

    ALOGI("Setting amplitude  to: %u", amplitude);
    mAmplitude << buf << std::endl;
    if (!mAmplitude) {
        ALOGE("Failed to set amplitude (%d): %s", errno, strerror(errno));
        return Status::UNKNOWN_ERROR;
    }
    return Status::OK;
}

static uint8_t convertEffectStrength(EffectStrength strength) {
    uint8_t amplitude;

    switch (strength) {
    case EffectStrength::LIGHT:
        amplitude = 18;
        break;
    case EffectStrength::MEDIUM:
        amplitude = 25;
        break;
    default:
    case EffectStrength::STRONG:
        amplitude = 31;
        break;
    }

    return amplitude;
}

Return<void> Vibrator::perform(Effect effect, EffectStrength strength, perform_cb _hidl_cb) {
    if (effect == Effect::CLICK) {
        on(CLICK_TIMING_MS);
        setAmplitude(convertEffectStrength(strength));
        _hidl_cb(Status::OK, CLICK_TIMING_MS);
    } else {
        _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);
    }
    return Void();
}

Return<void> Vibrator::perform_1_1(Effect_1_1 effect, EffectStrength strength,
        perform_cb _hidl_cb) {
    if (effect == Effect_1_1::TICK) {
        on(TICK_TIMING_MS);
        setAmplitude(convertEffectStrength(strength));
        _hidl_cb(Status::OK, TICK_TIMING_MS);
        return Void();
    } else if (effect < Effect_1_1::TICK) {
        return perform(static_cast<Effect>(effect), strength, _hidl_cb);
    } else {
        _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);
        return Void();
    }
}

} // namespace implementation
}  // namespace V1_1
}  // namespace vibrator
}  // namespace hardware
}  // namespace android
