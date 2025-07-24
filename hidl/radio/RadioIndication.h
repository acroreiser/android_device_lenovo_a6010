/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <android/hardware/radio/1.4/IRadioIndication.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace android::hardware::radio::implementation {

using ::android::sp;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;

struct RadioIndication : public V1_4::IRadioIndication {
    sp<V1_4::IRadioIndication> mRealRadioIndication;
    // Methods from ::android::hardware::radio::V1_0::IRadioIndication follow.
    Return<void> radioStateChanged(V1_0::RadioIndicationType type,
                                   V1_0::RadioState radioState) override;
    Return<void> callStateChanged(V1_0::RadioIndicationType type) override;
    Return<void> networkStateChanged(V1_0::RadioIndicationType type) override;
    Return<void> newSms(V1_0::RadioIndicationType type, const hidl_vec<uint8_t>& pdu) override;
    Return<void> newSmsStatusReport(V1_0::RadioIndicationType type,
                                    const hidl_vec<uint8_t>& pdu) override;
    Return<void> newSmsOnSim(V1_0::RadioIndicationType type, int32_t recordNumber) override;
    Return<void> onUssd(V1_0::RadioIndicationType type, V1_0::UssdModeType modeType,
                        const hidl_string& msg) override;
    Return<void> nitzTimeReceived(V1_0::RadioIndicationType type, const hidl_string& nitzTime,
                                  uint64_t receivedTime) override;
    Return<void> currentSignalStrength(V1_0::RadioIndicationType type,
                                       const V1_0::SignalStrength& signalStrength) override;
    Return<void> dataCallListChanged(V1_0::RadioIndicationType type,
                                     const hidl_vec<V1_0::SetupDataCallResult>& dcList) override;
    Return<void> suppSvcNotify(V1_0::RadioIndicationType type,
                               const V1_0::SuppSvcNotification& suppSvc) override;
    Return<void> stkSessionEnd(V1_0::RadioIndicationType type) override;
    Return<void> stkProactiveCommand(V1_0::RadioIndicationType type,
                                     const hidl_string& cmd) override;
    Return<void> stkEventNotify(V1_0::RadioIndicationType type, const hidl_string& cmd) override;
    Return<void> stkCallSetup(V1_0::RadioIndicationType type, int64_t timeout) override;
    Return<void> simSmsStorageFull(V1_0::RadioIndicationType type) override;
    Return<void> simRefresh(V1_0::RadioIndicationType type,
                            const V1_0::SimRefreshResult& refreshResult) override;
    Return<void> callRing(V1_0::RadioIndicationType type, bool isGsm,
                          const V1_0::CdmaSignalInfoRecord& record) override;
    Return<void> simStatusChanged(V1_0::RadioIndicationType type) override;
    Return<void> cdmaNewSms(V1_0::RadioIndicationType type,
                            const V1_0::CdmaSmsMessage& msg) override;
    Return<void> newBroadcastSms(V1_0::RadioIndicationType type,
                                 const hidl_vec<uint8_t>& data) override;
    Return<void> cdmaRuimSmsStorageFull(V1_0::RadioIndicationType type) override;
    Return<void> restrictedStateChanged(V1_0::RadioIndicationType type,
                                        V1_0::PhoneRestrictedState state) override;
    Return<void> enterEmergencyCallbackMode(V1_0::RadioIndicationType type) override;
    Return<void> cdmaCallWaiting(V1_0::RadioIndicationType type,
                                 const V1_0::CdmaCallWaiting& callWaitingRecord) override;
    Return<void> cdmaOtaProvisionStatus(V1_0::RadioIndicationType type,
                                        V1_0::CdmaOtaProvisionStatus status) override;
    Return<void> cdmaInfoRec(V1_0::RadioIndicationType type,
                             const V1_0::CdmaInformationRecords& records) override;
    Return<void> indicateRingbackTone(V1_0::RadioIndicationType type, bool start) override;
    Return<void> resendIncallMute(V1_0::RadioIndicationType type) override;
    Return<void> cdmaSubscriptionSourceChanged(V1_0::RadioIndicationType type,
                                               V1_0::CdmaSubscriptionSource cdmaSource) override;
    Return<void> cdmaPrlChanged(V1_0::RadioIndicationType type, int32_t version) override;
    Return<void> exitEmergencyCallbackMode(V1_0::RadioIndicationType type) override;
    Return<void> rilConnected(V1_0::RadioIndicationType type) override;
    Return<void> voiceRadioTechChanged(V1_0::RadioIndicationType type,
                                       V1_0::RadioTechnology rat) override;
    Return<void> cellInfoList(V1_0::RadioIndicationType type,
                              const hidl_vec<V1_0::CellInfo>& records) override;
    Return<void> imsNetworkStateChanged(V1_0::RadioIndicationType type) override;
    Return<void> subscriptionStatusChanged(V1_0::RadioIndicationType type, bool activate) override;
    Return<void> srvccStateNotify(V1_0::RadioIndicationType type, V1_0::SrvccState state) override;
    Return<void> hardwareConfigChanged(V1_0::RadioIndicationType type,
                                       const hidl_vec<V1_0::HardwareConfig>& configs) override;
    Return<void> radioCapabilityIndication(V1_0::RadioIndicationType type,
                                           const V1_0::RadioCapability& rc) override;
    Return<void> onSupplementaryServiceIndication(V1_0::RadioIndicationType type,
                                                  const V1_0::StkCcUnsolSsResult& ss) override;
    Return<void> stkCallControlAlphaNotify(V1_0::RadioIndicationType type,
                                           const hidl_string& alpha) override;
    Return<void> lceData(V1_0::RadioIndicationType type, const V1_0::LceDataInfo& lce) override;
    Return<void> pcoData(V1_0::RadioIndicationType type, const V1_0::PcoDataInfo& pco) override;
    Return<void> modemReset(V1_0::RadioIndicationType type, const hidl_string& reason) override;

    // Methods from ::android::hardware::radio::V1_1::IRadioIndication follow.
    Return<void> carrierInfoForImsiEncryption(V1_0::RadioIndicationType info) override;
    Return<void> networkScanResult(V1_0::RadioIndicationType type,
                                   const V1_1::NetworkScanResult& result) override;
    Return<void> keepaliveStatus(V1_0::RadioIndicationType type,
                                 const V1_1::KeepaliveStatus& status) override;

    // Methods from ::android::hardware::radio::V1_2::IRadioIndication follow.
    Return<void> networkScanResult_1_2(V1_0::RadioIndicationType type,
                                       const V1_2::NetworkScanResult& result) override;
    Return<void> cellInfoList_1_2(V1_0::RadioIndicationType type,
                                  const hidl_vec<V1_2::CellInfo>& records) override;
    Return<void> currentLinkCapacityEstimate(V1_0::RadioIndicationType type,
                                             const V1_2::LinkCapacityEstimate& lce) override;
    Return<void> currentPhysicalChannelConfigs(
            V1_0::RadioIndicationType type,
            const hidl_vec<V1_2::PhysicalChannelConfig>& configs) override;
    Return<void> currentSignalStrength_1_2(V1_0::RadioIndicationType type,
                                           const V1_2::SignalStrength& signalStrength) override;

    // Methods from ::android::hardware::radio::V1_4::IRadioIndication follow.
    Return<void> currentEmergencyNumberList(
            V1_0::RadioIndicationType type,
            const hidl_vec<V1_4::EmergencyNumber>& emergencyNumberList) override;
    Return<void> cellInfoList_1_4(V1_0::RadioIndicationType type,
                                  const hidl_vec<V1_4::CellInfo>& records) override;
    Return<void> networkScanResult_1_4(V1_0::RadioIndicationType type,
                                       const V1_4::NetworkScanResult& result) override;
    Return<void> currentPhysicalChannelConfigs_1_4(
            V1_0::RadioIndicationType type,
            const hidl_vec<V1_4::PhysicalChannelConfig>& configs) override;
    Return<void> dataCallListChanged_1_4(
            V1_0::RadioIndicationType type,
            const hidl_vec<V1_4::SetupDataCallResult>& dcList) override;
    Return<void> currentSignalStrength_1_4(V1_0::RadioIndicationType type,
                                           const V1_4::SignalStrength& signalStrength) override;
};

}  // namespace android::hardware::radio::implementation
