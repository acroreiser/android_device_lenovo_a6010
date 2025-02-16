/*
 * Copyright (C) 2019 The LineageOS Project
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

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/strings.h>

#include "KeyDisabler.h"

namespace vendor {
namespace lineage {
namespace touch {
namespace V1_0 {
namespace implementation {

constexpr const char kControlPath[] =
    "/sys/bus/i2c/drivers/ft5x06_ts/5-0038/disable_keys";

constexpr const char kControlPath2[] =
    "/sys/bus/i2c/drivers/mms200_i2c/5-0048/disable_keys";

KeyDisabler::KeyDisabler() {
    if (!access(kControlPath, F_OK) || !access(kControlPath2, F_OK))
        mHasKeyDisabler = true;
    else
        mHasKeyDisabler = false;
}

// Methods from ::vendor::lineage::touch::V1_0::IKeyDisabler follow.
Return<bool> KeyDisabler::isEnabled() {
    std::string buf;

    if (!mHasKeyDisabler) return false;

    if (!android::base::ReadFileToString(kControlPath, &buf) &&
        !android::base::ReadFileToString(kControlPath2, &buf)) {
        LOG(ERROR) << "Failed to read " << kControlPath;
        return false;
    }

    return std::stoi(android::base::Trim(buf)) == 0;
}

Return<bool> KeyDisabler::setEnabled(bool enabled) {
    if (!mHasKeyDisabler) return false;

    if(enabled == true) {
        android::base::WriteStringToFile("1", kControlPath);
        android::base::WriteStringToFile("1", kControlPath2);
    } else {
        android::base::WriteStringToFile("0", kControlPath);
        android::base::WriteStringToFile("0", kControlPath2);
    }

    return true;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace touch
}  // namespace lineage
}  // namespace vendor
