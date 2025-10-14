/*
 * Copyright (C) 2020-2021 The LineageOS Project
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

#define LOG_TAG "android.hardware.vibrator-service.msm8916"
#include <android-base/logging.h>

#include <android/binder_manager.h>
#include <android/binder_process.h>

#include "Vibrator.h"

using aidl::android::hardware::vibrator::Vibrator;
using aidl::android::hardware::vibrator::VIBRATOR_ENABLE_PATH;

std::shared_ptr<Vibrator> createVibratorService() {
    std::ofstream enable(VIBRATOR_ENABLE_PATH);
    if (!enable) {
        LOG(ERROR) << "Failed to open " << VIBRATOR_ENABLE_PATH << ". Error: " << errno << " - " << strerror(errno);
        return NULL;
    }

    return ndk::SharedRefBase::make<Vibrator>(std::move(enable));
}

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(0);
    std::shared_ptr<Vibrator> vibrator = createVibratorService();
    CHECK(vibrator != NULL);

    const std::string instance = std::string() + Vibrator::descriptor + "/default";
    binder_status_t status = AServiceManager_addService(vibrator->asBinder().get(), instance.c_str());
    CHECK(status == STATUS_OK);

    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE; // should not reach
}
