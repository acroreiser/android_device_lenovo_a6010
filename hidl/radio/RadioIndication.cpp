/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "RadioIndication.h"
#include "Helpers.h"

namespace android::hardware::radio::implementation {

// Methods from ::android::hardware::radio::V1_0::IRadioIndication follow.
Return<void> RadioIndication::radioStateChanged(V1_0::RadioIndicationType type,
                                                V1_0::RadioState radioState) {
    return mRealRadioIndication->radioStateChanged(type, radioState);
}

Return<void> RadioIndication::callStateChanged(V1_0::RadioIndicationType type) {
    return mRealRadioIndication->callStateChanged(type);
}

Return<void> RadioIndication::networkStateChanged(V1_0::RadioIndicationType type) {
    return mRealRadioIndication->networkStateChanged(type);
}

Return<void> RadioIndication::newSms(V1_0::RadioIndicationType type, const hidl_vec<uint8_t>& pdu) {
    return mRealRadioIndication->newSms(type, pdu);
}

Return<void> RadioIndication::newSmsStatusReport(V1_0::RadioIndicationType type,
                                                 const hidl_vec<uint8_t>& pdu) {
    return mRealRadioIndication->newSmsStatusReport(type, pdu);
}

Return<void> RadioIndication::newSmsOnSim(V1_0::RadioIndicationType type, int32_t recordNumber) {
    return mRealRadioIndication->newSmsOnSim(type, recordNumber);
}

Return<void> RadioIndication::onUssd(V1_0::RadioIndicationType type, V1_0::UssdModeType modeType,
                                     const hidl_string& msg) {
    return mRealRadioIndication->onUssd(type, modeType, msg);
}

Return<void> RadioIndication::nitzTimeReceived(V1_0::RadioIndicationType type,
                                               const hidl_string& nitzTime, uint64_t receivedTime) {
    return mRealRadioIndication->nitzTimeReceived(type, nitzTime, receivedTime);
}

Return<void> RadioIndication::currentSignalStrength(V1_0::RadioIndicationType type,
                                                    const V1_0::SignalStrength& signalStrength) {
    return mRealRadioIndication->currentSignalStrength_1_4(type, Create1_4SignalStrength(signalStrength));
}

Return<void> RadioIndication::dataCallListChanged(
        V1_0::RadioIndicationType type, const hidl_vec<V1_0::SetupDataCallResult>& dcList) {
    hidl_vec<V1_4::SetupDataCallResult> newDcList;
    newDcList.resize(dcList.size());
    for(int x = 0; x < dcList.size(); ++x)
        newDcList[x] = Create1_4SetupDataCallResult(dcList[x]);
    return mRealRadioIndication->dataCallListChanged_1_4(type, newDcList);
}

Return<void> RadioIndication::suppSvcNotify(V1_0::RadioIndicationType type,
                                            const V1_0::SuppSvcNotification& suppSvc) {
    return mRealRadioIndication->suppSvcNotify(type, suppSvc);
}

Return<void> RadioIndication::stkSessionEnd(V1_0::RadioIndicationType type) {
    return mRealRadioIndication->stkSessionEnd(type);
}

Return<void> RadioIndication::stkProactiveCommand(V1_0::RadioIndicationType type,
                                                  const hidl_string& cmd) {
    return mRealRadioIndication->stkProactiveCommand(type, cmd);
}

Return<void> RadioIndication::stkEventNotify(V1_0::RadioIndicationType type,
                                             const hidl_string& cmd) {
    return mRealRadioIndication->stkEventNotify(type, cmd);
}

Return<void> RadioIndication::stkCallSetup(V1_0::RadioIndicationType type, int64_t timeout) {
    return mRealRadioIndication->stkCallSetup(type, timeout);
}

Return<void> RadioIndication::simSmsStorageFull(V1_0::RadioIndicationType type) {
    return mRealRadioIndication->simSmsStorageFull(type);
}

Return<void> RadioIndication::simRefresh(V1_0::RadioIndicationType type,
                                         const V1_0::SimRefreshResult& refreshResult) {
    return mRealRadioIndication->simRefresh(type, refreshResult);
}

Return<void> RadioIndication::callRing(V1_0::RadioIndicationType type, bool isGsm,
                                       const V1_0::CdmaSignalInfoRecord& record) {
    return mRealRadioIndication->callRing(type, isGsm, record);
}

Return<void> RadioIndication::simStatusChanged(V1_0::RadioIndicationType type) {
    return mRealRadioIndication->simStatusChanged(type);
}

Return<void> RadioIndication::cdmaNewSms(V1_0::RadioIndicationType type,
                                         const V1_0::CdmaSmsMessage& msg) {
    return mRealRadioIndication->cdmaNewSms(type, msg);
}

Return<void> RadioIndication::newBroadcastSms(V1_0::RadioIndicationType type,
                                              const hidl_vec<uint8_t>& data) {
    return mRealRadioIndication->newBroadcastSms(type, data);
}

Return<void> RadioIndication::cdmaRuimSmsStorageFull(V1_0::RadioIndicationType type) {
    return mRealRadioIndication->cdmaRuimSmsStorageFull(type);
}

Return<void> RadioIndication::restrictedStateChanged(V1_0::RadioIndicationType type,
                                                     V1_0::PhoneRestrictedState state) {
    return mRealRadioIndication->restrictedStateChanged(type, state);
}

Return<void> RadioIndication::enterEmergencyCallbackMode(V1_0::RadioIndicationType type) {
    return mRealRadioIndication->enterEmergencyCallbackMode(type);
}

Return<void> RadioIndication::cdmaCallWaiting(V1_0::RadioIndicationType type,
                                              const V1_0::CdmaCallWaiting& callWaitingRecord) {
    return mRealRadioIndication->cdmaCallWaiting(type, callWaitingRecord);
}

Return<void> RadioIndication::cdmaOtaProvisionStatus(V1_0::RadioIndicationType type,
                                                     V1_0::CdmaOtaProvisionStatus status) {
    return mRealRadioIndication->cdmaOtaProvisionStatus(type, status);
}

Return<void> RadioIndication::cdmaInfoRec(V1_0::RadioIndicationType type,
                                          const V1_0::CdmaInformationRecords& records) {
    return mRealRadioIndication->cdmaInfoRec(type, records);
}

Return<void> RadioIndication::indicateRingbackTone(V1_0::RadioIndicationType type, bool start) {
    return mRealRadioIndication->indicateRingbackTone(type, start);
}

Return<void> RadioIndication::resendIncallMute(V1_0::RadioIndicationType type) {
    return mRealRadioIndication->resendIncallMute(type);
}

Return<void> RadioIndication::cdmaSubscriptionSourceChanged(
        V1_0::RadioIndicationType type, V1_0::CdmaSubscriptionSource cdmaSource) {
    return mRealRadioIndication->cdmaSubscriptionSourceChanged(type, cdmaSource);
}

Return<void> RadioIndication::cdmaPrlChanged(V1_0::RadioIndicationType type, int32_t version) {
    return mRealRadioIndication->cdmaPrlChanged(type, version);
}

Return<void> RadioIndication::exitEmergencyCallbackMode(V1_0::RadioIndicationType type) {
    return mRealRadioIndication->exitEmergencyCallbackMode(type);
}

Return<void> RadioIndication::rilConnected(V1_0::RadioIndicationType type) {
    return mRealRadioIndication->rilConnected(type);
}

Return<void> RadioIndication::voiceRadioTechChanged(V1_0::RadioIndicationType type,
                                                    V1_0::RadioTechnology rat) {
    return mRealRadioIndication->voiceRadioTechChanged(type, rat);
}

Return<void> RadioIndication::cellInfoList(V1_0::RadioIndicationType type,
                                           const hidl_vec<V1_0::CellInfo>& records) {
    return mRealRadioIndication->cellInfoList_1_4(type, Create1_4CellInfoList(records));
}

Return<void> RadioIndication::imsNetworkStateChanged(V1_0::RadioIndicationType type) {
    return mRealRadioIndication->imsNetworkStateChanged(type);
}

Return<void> RadioIndication::subscriptionStatusChanged(V1_0::RadioIndicationType type,
                                                        bool activate) {
    return mRealRadioIndication->subscriptionStatusChanged(type, activate);
}

Return<void> RadioIndication::srvccStateNotify(V1_0::RadioIndicationType type,
                                               V1_0::SrvccState state) {
    return mRealRadioIndication->srvccStateNotify(type, state);
}

Return<void> RadioIndication::hardwareConfigChanged(V1_0::RadioIndicationType type,
                                                    const hidl_vec<V1_0::HardwareConfig>& configs) {
    return mRealRadioIndication->hardwareConfigChanged(type, configs);
}

Return<void> RadioIndication::radioCapabilityIndication(V1_0::RadioIndicationType type,
                                                        const V1_0::RadioCapability& rc) {
    return mRealRadioIndication->radioCapabilityIndication(type, rc);
}

Return<void> RadioIndication::onSupplementaryServiceIndication(V1_0::RadioIndicationType type,
                                                               const V1_0::StkCcUnsolSsResult& ss) {
    return mRealRadioIndication->onSupplementaryServiceIndication(type, ss);
}

Return<void> RadioIndication::stkCallControlAlphaNotify(V1_0::RadioIndicationType type,
                                                        const hidl_string& alpha) {
    return mRealRadioIndication->stkCallControlAlphaNotify(type, alpha);
}

Return<void> RadioIndication::lceData(V1_0::RadioIndicationType type,
                                      const V1_0::LceDataInfo& lce) {
    return mRealRadioIndication->lceData(type, lce);
}

Return<void> RadioIndication::pcoData(V1_0::RadioIndicationType type,
                                      const V1_0::PcoDataInfo& pco) {
    return mRealRadioIndication->pcoData(type, pco);
}

Return<void> RadioIndication::modemReset(V1_0::RadioIndicationType type,
                                         const hidl_string& reason) {
    return mRealRadioIndication->modemReset(type, reason);
}

// Methods from ::android::hardware::radio::V1_1::IRadioIndication follow.
Return<void> RadioIndication::carrierInfoForImsiEncryption(V1_0::RadioIndicationType info) {
    return mRealRadioIndication->carrierInfoForImsiEncryption(info);
}

Return<void> RadioIndication::networkScanResult(V1_0::RadioIndicationType type,
                                                const V1_1::NetworkScanResult& result) {
    V1_4::NetworkScanResult newNSR = {};
    newNSR.status = result.status;
    newNSR.error = result.error;
    newNSR.networkInfos = Create1_4CellInfoList(result.networkInfos);
    return mRealRadioIndication->networkScanResult_1_4(type, newNSR);
}

Return<void> RadioIndication::keepaliveStatus(V1_0::RadioIndicationType type,
                                              const V1_1::KeepaliveStatus& status) {
    return mRealRadioIndication->keepaliveStatus(type, status);
}

// Methods from ::android::hardware::radio::V1_2::IRadioIndication follow.
Return<void> RadioIndication::networkScanResult_1_2(V1_0::RadioIndicationType type,
                                                    const V1_2::NetworkScanResult& result) {
    V1_4::NetworkScanResult newNSR = {};
    newNSR.status = result.status;
    newNSR.error = result.error;
    newNSR.networkInfos = Create1_4CellInfoList(result.networkInfos);
    return mRealRadioIndication->networkScanResult_1_4(type, newNSR);
}

Return<void> RadioIndication::cellInfoList_1_2(V1_0::RadioIndicationType type,
                                               const hidl_vec<V1_2::CellInfo>& records) {
    return mRealRadioIndication->cellInfoList_1_4(type, Create1_4CellInfoList(records));
}

Return<void> RadioIndication::currentLinkCapacityEstimate(V1_0::RadioIndicationType type,
                                                          const V1_2::LinkCapacityEstimate& lce) {
    return mRealRadioIndication->currentLinkCapacityEstimate(type, lce);
}

Return<void> RadioIndication::currentPhysicalChannelConfigs(
        V1_0::RadioIndicationType type, const hidl_vec<V1_2::PhysicalChannelConfig>& configs) {
    hidl_vec<V1_4::PhysicalChannelConfig> newConfigs;
    newConfigs.resize(configs.size());
    for(int x = 0; x < configs.size(); ++x){
        newConfigs[x].base = configs[x];
        newConfigs[x].rat = V1_4::RadioTechnology::UNKNOWN;
        newConfigs[x].physicalCellId = -1;
    }
    return mRealRadioIndication->currentPhysicalChannelConfigs_1_4(type, newConfigs);
}

Return<void> RadioIndication::currentSignalStrength_1_2(
        V1_0::RadioIndicationType type, const V1_2::SignalStrength& signalStrength) {
    return mRealRadioIndication->currentSignalStrength_1_4(type, Create1_4SignalStrength(signalStrength));
}

// Methods from ::android::hardware::radio::V1_4::IRadioIndication follow.
Return<void> RadioIndication::currentEmergencyNumberList(
        V1_0::RadioIndicationType type,
        const hidl_vec<V1_4::EmergencyNumber>& emergencyNumberList) {
    return mRealRadioIndication->currentEmergencyNumberList(type, emergencyNumberList);
}

Return<void> RadioIndication::cellInfoList_1_4(V1_0::RadioIndicationType type,
                                               const hidl_vec<V1_4::CellInfo>& records) {
    return mRealRadioIndication->cellInfoList_1_4(type, records);
}

Return<void> RadioIndication::networkScanResult_1_4(V1_0::RadioIndicationType type,
                                                    const V1_4::NetworkScanResult& result) {
    return mRealRadioIndication->networkScanResult_1_4(type, result);
}

Return<void> RadioIndication::currentPhysicalChannelConfigs_1_4(
        V1_0::RadioIndicationType type, const hidl_vec<V1_4::PhysicalChannelConfig>& configs) {
    return mRealRadioIndication->currentPhysicalChannelConfigs_1_4(type, configs);
}

Return<void> RadioIndication::dataCallListChanged_1_4(
        V1_0::RadioIndicationType type, const hidl_vec<V1_4::SetupDataCallResult>& dcList) {
    return mRealRadioIndication->dataCallListChanged_1_4(type, dcList);
}

Return<void> RadioIndication::currentSignalStrength_1_4(
        V1_0::RadioIndicationType type, const V1_4::SignalStrength& signalStrength) {
    return mRealRadioIndication->currentSignalStrength_1_4(type, signalStrength);
}

}  // namespace android::hardware::radio::implementation
