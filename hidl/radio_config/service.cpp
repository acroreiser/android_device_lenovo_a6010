/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.1
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "android.hardware.radio.config@1.1-service-hh"

#include <android/hardware/radio/config/1.1/IRadioConfig.h>
#include <hidl/HidlTransportSupport.h>

#include "RadioConfig.h"

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using namespace android::hardware::radio::config;

using android::sp;
using android::status_t;
using android::OK;

int main() {
    configureRpcThreadpool(1, true);

    sp<V1_1::IRadioConfig> radioConfig = new RadioConfig;
    status_t status = radioConfig->registerAsService();
    ALOGW_IF(status != OK, "Could not register IRadioConfig");
    ALOGD("Default service is ready.");

    joinRpcThreadpool();
    return 0;
}