/*
 * Copyright (C) 2018 The LineageOS Project
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

#define LOG_TAG "LightService"

#include "Light.h"

#include <android-base/logging.h>

namespace {
using android::hardware::light::V2_0::LightState;

static constexpr int DEFAULT_MAX_BRIGHTNESS = 255;

static uint32_t rgbToBrightness(const LightState& state) {
    uint32_t color = state.color & 0x00ffffff;
    return ((77 * ((color >> 16) & 0xff)) + (150 * ((color >> 8) & 0xff)) +
            (29 * (color & 0xff))) >> 8;
}

static bool isLit(const LightState& state) {
    return (state.color & 0x00ffffff);
}
}  // anonymous namespace

namespace android {
namespace hardware {
namespace light {
namespace V2_0 {
namespace implementation {

static uint32_t green_status;
static uint32_t red_status;
static uint32_t init_status;

Light::Light(std::pair<std::ofstream, uint32_t>&& lcd_backlight,
             std::ofstream&& charging_led,
             std::ofstream&& notification_led)
    : mLcdBacklight(std::move(lcd_backlight)),
      mChargingLed(std::move(charging_led)),
      mNotificationLed(std::move(notification_led)) {
    auto backlightFn(std::bind(&Light::setLcdBacklight, this, std::placeholders::_1));
    auto batteryFn(std::bind(&Light::setBatteryLight, this, std::placeholders::_1));
    auto notificationFn(std::bind(&Light::setNotificationLight, this, std::placeholders::_1));
    auto attentionFn(std::bind(&Light::setNotificationLight, this, std::placeholders::_1));
    mLights.emplace(std::make_pair(Type::BACKLIGHT, backlightFn));
    mLights.emplace(std::make_pair(Type::BATTERY, batteryFn));
    mLights.emplace(std::make_pair(Type::NOTIFICATIONS, notificationFn));
    mLights.emplace(std::make_pair(Type::ATTENTION, attentionFn));
}

// Methods from ::android::hardware::light::V2_0::ILight follow.
Return<Status> Light::setLight(Type type, const LightState& state) {
    auto it = mLights.find(type);

    if (it == mLights.end()) {
        return Status::LIGHT_NOT_SUPPORTED;
    }

    it->second(state);

    return Status::SUCCESS;
}

Return<void> Light::getSupportedTypes(getSupportedTypes_cb _hidl_cb) {
    std::vector<Type> types;
    for (auto const& light : mLights) {
        types.push_back(light.first);
    }

    _hidl_cb(types);

    return Void();
}

void Light::setLcdBacklight(const LightState& state) {
    std::lock_guard<std::mutex> lock(mLock);

    uint32_t brightness = rgbToBrightness(state);

    // If max panel brightness is not the default (255),
    // apply linear scaling across the accepted range.
    if (mLcdBacklight.second != DEFAULT_MAX_BRIGHTNESS) {
        int old_brightness = brightness;
        brightness = brightness * mLcdBacklight.second / DEFAULT_MAX_BRIGHTNESS;
        LOG(VERBOSE) << "scaling brightness " << old_brightness << " => " << brightness;
    }

    mLcdBacklight.first << brightness << std::endl;
}

void Light::setBatteryLight(const LightState& state) {
    std::lock_guard<std::mutex> lock(mLock);
    mBatteryState = state;
    setSpeakerBatteryLightLocked();
}

void Light::setSpeakerBatteryLightLocked() {
    setSpeakerLightLocked(mBatteryState);
}

void Light::setSpeakerLightLocked(const LightState& state) {
    if (isLit(state)) {
        if(green_status == 1)
            mNotificationLed << "timer" << std::endl;
        else
            mChargingLed << "timer" << std::endl;

        red_status = 1;
    } else {
        // Lights off
        if(green_status == 1)
            mNotificationLed << "default-on" << std::endl;
        else
            mChargingLed << "none" << std::endl;

        red_status = 0;
    }
}

void Light::setNotificationLight(const LightState& state) {
    std::lock_guard<std::mutex> lock(mLock);
    mNotificationState = state;
    setSpeakerNotificationLightLocked();
}

void Light::setSpeakerNotificationLightLocked() {
    setSpeakerNLightLocked(mNotificationState);
}

void Light::setSpeakerNLightLocked(const LightState& state) {
    if (isLit(state)) {
        if(red_status == 1)
        {
            mChargingLed << "none" << std::endl;
            mNotificationLed << "timer" << std::endl;
        }
        else
            mNotificationLed << "default-on" << std::endl;

        green_status = 1;
    } else {
        // avoid disabling green led at boot stage, only don't blink
        if (init_status < 1)
        {
            mNotificationLed << "default-on" << std::endl;
            green_status = 0;
            init_status = 1;
            return;
        }

        mNotificationLed << "none" << std::endl;
        if (red_status == 1)
            mChargingLed << "timer" << std::endl;

        green_status = 0;
    }
}

}  // namespace implementation
}  // namespace V2_0
}  // namespace light
}  // namespace hardware
}  // namespace android
