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

#include "RadioConfig.h"

namespace android {
namespace hardware {
namespace radio {
namespace config {

using namespace ::android::hardware::radio::config::V1_1;

// Methods from ::android::hardware::radio::config::V1_1::IRadioConfig follow.
Return<void> RadioConfig::setResponseFunctions(
    const sp<V1_0::IRadioConfigResponse>& radioConfigResponse,
    const sp<V1_0::IRadioConfigIndication>& radioConfigIndication) {
    mRadioConfigResponse = V1_1::IRadioConfigResponse::castFrom(radioConfigResponse);
    mRadioConfigIndication = radioConfigIndication;
    return Void();
}

Return<void> RadioConfig::getSimSlotsStatus(int32_t /* serial */) {
    hidl_vec<V1_0::SimSlotStatus> slotStatus;
    ::android::hardware::radio::V1_0::RadioResponseInfo info;
    mRadioConfigResponse->getSimSlotsStatusResponse(info, slotStatus);
    return Void();
}

Return<void> RadioConfig::setSimSlotsMapping(int32_t /* serial */,
                                             const hidl_vec<uint32_t>& /* slotMap */) {
    ::android::hardware::radio::V1_0::RadioResponseInfo info;
    mRadioConfigResponse->setSimSlotsMappingResponse(info);
    return Void();
}

Return<void> RadioConfig::getPhoneCapability(int32_t /* serial */) {
    ::android::hardware::radio::V1_0::RadioResponseInfo info;
    ::android::hardware::radio::config::V1_1::PhoneCapability capability;
    mRadioConfigResponse->getPhoneCapabilityResponse(info, capability);
    return Void();
}

Return<void> RadioConfig::setPreferredDataModem(int32_t /* serial */, uint8_t /* modemId */) {
    ::android::hardware::radio::V1_0::RadioResponseInfo info;
    mRadioConfigResponse->setPreferredDataModemResponse(info);
    return Void();
}

Return<void> RadioConfig::setModemsConfig(int32_t /* serial */, const ::android::hardware::radio::config::V1_1::ModemsConfig& /* modemsConfig */) {
    ::android::hardware::radio::V1_0::RadioResponseInfo info;
    mRadioConfigResponse->setModemsConfigResponse(info);
    return Void();
}

Return<void> RadioConfig::getModemsConfig(int32_t /* serial */) {
    ::android::hardware::radio::V1_0::RadioResponseInfo info;
    const ::android::hardware::radio::config::V1_1::ModemsConfig config = {};
    mRadioConfigResponse->getModemsConfigResponse(info, config);
    return Void();
}

}  // namespace config
}  // namespace radio
}  // namespace hardware
}  // namespace android
