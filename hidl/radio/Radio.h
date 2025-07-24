/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <android/hardware/radio/1.4/IRadio.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#include "RadioIndication.h"
#include "RadioResponse.h"

namespace android::hardware::radio::implementation {

using ::android::sp;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;

struct Radio : public V1_4::IRadio {
  public:
    Radio(sp<V1_0::IRadio> realRadio);

    // Methods from ::android::hardware::radio::V1_0::IRadio follow.
    Return<void> setResponseFunctions(const sp<V1_0::IRadioResponse>& radioResponse,
                                      const sp<V1_0::IRadioIndication>& radioIndication) override;
    Return<void> getIccCardStatus(int32_t serial) override;
    Return<void> supplyIccPinForApp(int32_t serial, const hidl_string& pin,
                                    const hidl_string& aid) override;
    Return<void> supplyIccPukForApp(int32_t serial, const hidl_string& puk, const hidl_string& pin,
                                    const hidl_string& aid) override;
    Return<void> supplyIccPin2ForApp(int32_t serial, const hidl_string& pin2,
                                     const hidl_string& aid) override;
    Return<void> supplyIccPuk2ForApp(int32_t serial, const hidl_string& puk2,
                                     const hidl_string& pin2, const hidl_string& aid) override;
    Return<void> changeIccPinForApp(int32_t serial, const hidl_string& oldPin,
                                    const hidl_string& newPin, const hidl_string& aid) override;
    Return<void> changeIccPin2ForApp(int32_t serial, const hidl_string& oldPin2,
                                     const hidl_string& newPin2, const hidl_string& aid) override;
    Return<void> supplyNetworkDepersonalization(int32_t serial, const hidl_string& netPin) override;
    Return<void> getCurrentCalls(int32_t serial) override;
    Return<void> dial(int32_t serial, const V1_0::Dial& dialInfo) override;
    Return<void> getImsiForApp(int32_t serial, const hidl_string& aid) override;
    Return<void> hangup(int32_t serial, int32_t gsmIndex) override;
    Return<void> hangupWaitingOrBackground(int32_t serial) override;
    Return<void> hangupForegroundResumeBackground(int32_t serial) override;
    Return<void> switchWaitingOrHoldingAndActive(int32_t serial) override;
    Return<void> conference(int32_t serial) override;
    Return<void> rejectCall(int32_t serial) override;
    Return<void> getLastCallFailCause(int32_t serial) override;
    Return<void> getSignalStrength(int32_t serial) override;
    Return<void> getVoiceRegistrationState(int32_t serial) override;
    Return<void> getDataRegistrationState(int32_t serial) override;
    Return<void> getOperator(int32_t serial) override;
    Return<void> setRadioPower(int32_t serial, bool on) override;
    Return<void> sendDtmf(int32_t serial, const hidl_string& s) override;
    Return<void> sendSms(int32_t serial, const V1_0::GsmSmsMessage& message) override;
    Return<void> sendSMSExpectMore(int32_t serial, const V1_0::GsmSmsMessage& message) override;
    Return<void> setupDataCall(int32_t serial, V1_0::RadioTechnology radioTechnology,
                               const V1_0::DataProfileInfo& dataProfileInfo, bool modemCognitive,
                               bool roamingAllowed, bool isRoaming) override;
    Return<void> iccIOForApp(int32_t serial, const V1_0::IccIo& iccIo) override;
    Return<void> sendUssd(int32_t serial, const hidl_string& ussd) override;
    Return<void> cancelPendingUssd(int32_t serial) override;
    Return<void> getClir(int32_t serial) override;
    Return<void> setClir(int32_t serial, int32_t status) override;
    Return<void> getCallForwardStatus(int32_t serial,
                                      const V1_0::CallForwardInfo& callInfo) override;
    Return<void> setCallForward(int32_t serial, const V1_0::CallForwardInfo& callInfo) override;
    Return<void> getCallWaiting(int32_t serial, int32_t serviceClass) override;
    Return<void> setCallWaiting(int32_t serial, bool enable, int32_t serviceClass) override;
    Return<void> acknowledgeLastIncomingGsmSms(int32_t serial, bool success,
                                               V1_0::SmsAcknowledgeFailCause cause) override;
    Return<void> acceptCall(int32_t serial) override;
    Return<void> deactivateDataCall(int32_t serial, int32_t cid, bool reasonRadioShutDown) override;
    Return<void> getFacilityLockForApp(int32_t serial, const hidl_string& facility,
                                       const hidl_string& password, int32_t serviceClass,
                                       const hidl_string& appId) override;
    Return<void> setFacilityLockForApp(int32_t serial, const hidl_string& facility, bool lockState,
                                       const hidl_string& password, int32_t serviceClass,
                                       const hidl_string& appId) override;
    Return<void> setBarringPassword(int32_t serial, const hidl_string& facility,
                                    const hidl_string& oldPassword,
                                    const hidl_string& newPassword) override;
    Return<void> getNetworkSelectionMode(int32_t serial) override;
    Return<void> setNetworkSelectionModeAutomatic(int32_t serial) override;
    Return<void> setNetworkSelectionModeManual(int32_t serial,
                                               const hidl_string& operatorNumeric) override;
    Return<void> getAvailableNetworks(int32_t serial) override;
    Return<void> startDtmf(int32_t serial, const hidl_string& s) override;
    Return<void> stopDtmf(int32_t serial) override;
    Return<void> getBasebandVersion(int32_t serial) override;
    Return<void> separateConnection(int32_t serial, int32_t gsmIndex) override;
    Return<void> setMute(int32_t serial, bool enable) override;
    Return<void> getMute(int32_t serial) override;
    Return<void> getClip(int32_t serial) override;
    Return<void> getDataCallList(int32_t serial) override;
    Return<void> setSuppServiceNotifications(int32_t serial, bool enable) override;
    Return<void> writeSmsToSim(int32_t serial, const V1_0::SmsWriteArgs& smsWriteArgs) override;
    Return<void> deleteSmsOnSim(int32_t serial, int32_t index) override;
    Return<void> setBandMode(int32_t serial, V1_0::RadioBandMode mode) override;
    Return<void> getAvailableBandModes(int32_t serial) override;
    Return<void> sendEnvelope(int32_t serial, const hidl_string& command) override;
    Return<void> sendTerminalResponseToSim(int32_t serial,
                                           const hidl_string& commandResponse) override;
    Return<void> handleStkCallSetupRequestFromSim(int32_t serial, bool accept) override;
    Return<void> explicitCallTransfer(int32_t serial) override;
    Return<void> setPreferredNetworkType(int32_t serial,
                                         V1_0::PreferredNetworkType nwType) override;
    Return<void> getPreferredNetworkType(int32_t serial) override;
    Return<void> getNeighboringCids(int32_t serial) override;
    Return<void> setLocationUpdates(int32_t serial, bool enable) override;
    Return<void> setCdmaSubscriptionSource(int32_t serial,
                                           V1_0::CdmaSubscriptionSource cdmaSub) override;
    Return<void> setCdmaRoamingPreference(int32_t serial, V1_0::CdmaRoamingType type) override;
    Return<void> getCdmaRoamingPreference(int32_t serial) override;
    Return<void> setTTYMode(int32_t serial, V1_0::TtyMode mode) override;
    Return<void> getTTYMode(int32_t serial) override;
    Return<void> setPreferredVoicePrivacy(int32_t serial, bool enable) override;
    Return<void> getPreferredVoicePrivacy(int32_t serial) override;
    Return<void> sendCDMAFeatureCode(int32_t serial, const hidl_string& featureCode) override;
    Return<void> sendBurstDtmf(int32_t serial, const hidl_string& dtmf, int32_t on,
                               int32_t off) override;
    Return<void> sendCdmaSms(int32_t serial, const V1_0::CdmaSmsMessage& sms) override;
    Return<void> acknowledgeLastIncomingCdmaSms(int32_t serial,
                                                const V1_0::CdmaSmsAck& smsAck) override;
    Return<void> getGsmBroadcastConfig(int32_t serial) override;
    Return<void> setGsmBroadcastConfig(
            int32_t serial, const hidl_vec<V1_0::GsmBroadcastSmsConfigInfo>& configInfo) override;
    Return<void> setGsmBroadcastActivation(int32_t serial, bool activate) override;
    Return<void> getCdmaBroadcastConfig(int32_t serial) override;
    Return<void> setCdmaBroadcastConfig(
            int32_t serial, const hidl_vec<V1_0::CdmaBroadcastSmsConfigInfo>& configInfo) override;
    Return<void> setCdmaBroadcastActivation(int32_t serial, bool activate) override;
    Return<void> getCDMASubscription(int32_t serial) override;
    Return<void> writeSmsToRuim(int32_t serial, const V1_0::CdmaSmsWriteArgs& cdmaSms) override;
    Return<void> deleteSmsOnRuim(int32_t serial, int32_t index) override;
    Return<void> getDeviceIdentity(int32_t serial) override;
    Return<void> exitEmergencyCallbackMode(int32_t serial) override;
    Return<void> getSmscAddress(int32_t serial) override;
    Return<void> setSmscAddress(int32_t serial, const hidl_string& smsc) override;
    Return<void> reportSmsMemoryStatus(int32_t serial, bool available) override;
    Return<void> reportStkServiceIsRunning(int32_t serial) override;
    Return<void> getCdmaSubscriptionSource(int32_t serial) override;
    Return<void> requestIsimAuthentication(int32_t serial, const hidl_string& challenge) override;
    Return<void> acknowledgeIncomingGsmSmsWithPdu(int32_t serial, bool success,
                                                  const hidl_string& ackPdu) override;
    Return<void> sendEnvelopeWithStatus(int32_t serial, const hidl_string& contents) override;
    Return<void> getVoiceRadioTechnology(int32_t serial) override;
    Return<void> getCellInfoList(int32_t serial) override;
    Return<void> setCellInfoListRate(int32_t serial, int32_t rate) override;
    Return<void> setInitialAttachApn(int32_t serial, const V1_0::DataProfileInfo& dataProfileInfo,
                                     bool modemCognitive, bool isRoaming) override;
    Return<void> getImsRegistrationState(int32_t serial) override;
    Return<void> sendImsSms(int32_t serial, const V1_0::ImsSmsMessage& message) override;
    Return<void> iccTransmitApduBasicChannel(int32_t serial, const V1_0::SimApdu& message) override;
    Return<void> iccOpenLogicalChannel(int32_t serial, const hidl_string& aid, int32_t p2) override;
    Return<void> iccCloseLogicalChannel(int32_t serial, int32_t channelId) override;
    Return<void> iccTransmitApduLogicalChannel(int32_t serial,
                                               const V1_0::SimApdu& message) override;
    Return<void> nvReadItem(int32_t serial, V1_0::NvItem itemId) override;
    Return<void> nvWriteItem(int32_t serial, const V1_0::NvWriteItem& item) override;
    Return<void> nvWriteCdmaPrl(int32_t serial, const hidl_vec<uint8_t>& prl) override;
    Return<void> nvResetConfig(int32_t serial, V1_0::ResetNvType resetType) override;
    Return<void> setUiccSubscription(int32_t serial, const V1_0::SelectUiccSub& uiccSub) override;
    Return<void> setDataAllowed(int32_t serial, bool allow) override;
    Return<void> getHardwareConfig(int32_t serial) override;
    Return<void> requestIccSimAuthentication(int32_t serial, int32_t authContext,
                                             const hidl_string& authData,
                                             const hidl_string& aid) override;
    Return<void> setDataProfile(int32_t serial, const hidl_vec<V1_0::DataProfileInfo>& profiles,
                                bool isRoaming) override;
    Return<void> requestShutdown(int32_t serial) override;
    Return<void> getRadioCapability(int32_t serial) override;
    Return<void> setRadioCapability(int32_t serial, const V1_0::RadioCapability& rc) override;
    Return<void> startLceService(int32_t serial, int32_t reportInterval, bool pullMode) override;
    Return<void> stopLceService(int32_t serial) override;
    Return<void> pullLceData(int32_t serial) override;
    Return<void> getModemActivityInfo(int32_t serial) override;
    Return<void> setAllowedCarriers(int32_t serial, bool allAllowed,
                                    const V1_0::CarrierRestrictions& carriers) override;
    Return<void> getAllowedCarriers(int32_t serial) override;
    Return<void> sendDeviceState(int32_t serial, V1_0::DeviceStateType deviceStateType,
                                 bool state) override;
    Return<void> setIndicationFilter(
            int32_t serial, hidl_bitfield<V1_0::IndicationFilter> indicationFilter) override;
    Return<void> setSimCardPower(int32_t serial, bool powerUp) override;
    Return<void> responseAcknowledgement() override;

    // Methods from ::android::hardware::radio::V1_1::IRadio follow.
    Return<void> setCarrierInfoForImsiEncryption(
            int32_t serial, const V1_1::ImsiEncryptionInfo& imsiEncryptionInfo) override;
    Return<void> setSimCardPower_1_1(int32_t serial, V1_1::CardPowerState powerUp) override;
    Return<void> startNetworkScan(int32_t serial, const V1_1::NetworkScanRequest& request) override;
    Return<void> stopNetworkScan(int32_t serial) override;
    Return<void> startKeepalive(int32_t serial, const V1_1::KeepaliveRequest& keepalive) override;
    Return<void> stopKeepalive(int32_t serial, int32_t sessionHandle) override;

    // Methods from ::android::hardware::radio::V1_2::IRadio follow.
    Return<void> startNetworkScan_1_2(int32_t serial,
                                      const V1_2::NetworkScanRequest& request) override;
    Return<void> setIndicationFilter_1_2(
            int32_t serial, hidl_bitfield<V1_2::IndicationFilter> indicationFilter) override;
    Return<void> setSignalStrengthReportingCriteria(int32_t serial, int32_t hysteresisMs,
                                                    int32_t hysteresisDb,
                                                    const hidl_vec<int32_t>& thresholdsDbm,
                                                    V1_2::AccessNetwork accessNetwork) override;
    Return<void> setLinkCapacityReportingCriteria(int32_t serial, int32_t hysteresisMs,
                                                  int32_t hysteresisDlKbps,
                                                  int32_t hysteresisUlKbps,
                                                  const hidl_vec<int32_t>& thresholdsDownlinkKbps,
                                                  const hidl_vec<int32_t>& thresholdsUplinkKbps,
                                                  V1_2::AccessNetwork accessNetwork) override;
    Return<void> setupDataCall_1_2(int32_t serial, V1_2::AccessNetwork accessNetwork,
                                   const V1_0::DataProfileInfo& dataProfileInfo,
                                   bool modemCognitive, bool roamingAllowed, bool isRoaming,
                                   V1_2::DataRequestReason reason,
                                   const hidl_vec<hidl_string>& addresses,
                                   const hidl_vec<hidl_string>& dnses) override;
    Return<void> deactivateDataCall_1_2(int32_t serial, int32_t cid,
                                        V1_2::DataRequestReason reason) override;

    // Methods from ::android::hardware::radio::V1_3::IRadio follow.
    Return<void> setSystemSelectionChannels(
            int32_t serial, bool specifyChannels,
            const hidl_vec<V1_1::RadioAccessSpecifier>& specifiers) override;
    Return<void> enableModem(int32_t serial, bool on) override;
    Return<void> getModemStackStatus(int32_t serial) override;

    // Methods from ::android::hardware::radio::V1_4::IRadio follow.
    Return<void> setupDataCall_1_4(int32_t serial, V1_4::AccessNetwork accessNetwork,
                                   const V1_4::DataProfileInfo& dataProfileInfo,
                                   bool roamingAllowed, V1_2::DataRequestReason reason,
                                   const hidl_vec<hidl_string>& addresses,
                                   const hidl_vec<hidl_string>& dnses) override;
    Return<void> setInitialAttachApn_1_4(int32_t serial,
                                         const V1_4::DataProfileInfo& dataProfileInfo) override;
    Return<void> setDataProfile_1_4(int32_t serial,
                                    const hidl_vec<V1_4::DataProfileInfo>& profiles) override;
    Return<void> emergencyDial(int32_t serial, const V1_0::Dial& dialInfo,
                               hidl_bitfield<V1_4::EmergencyServiceCategory> categories,
                               const hidl_vec<hidl_string>& urns,
                               V1_4::EmergencyCallRouting routing, bool hasKnownUserIntentEmergency,
                               bool isTesting) override;
    Return<void> startNetworkScan_1_4(int32_t serial,
                                      const V1_2::NetworkScanRequest& request) override;
    Return<void> getPreferredNetworkTypeBitmap(int32_t serial) override;
    Return<void> setPreferredNetworkTypeBitmap(
            int32_t serial, hidl_bitfield<V1_4::RadioAccessFamily> networkTypeBitmap) override;
    Return<void> setAllowedCarriers_1_4(int32_t serial,
                                        const V1_4::CarrierRestrictionsWithPriority& carriers,
                                        V1_4::SimLockMultiSimPolicy multiSimPolicy) override;
    Return<void> getAllowedCarriers_1_4(int32_t serial) override;
    Return<void> getSignalStrength_1_4(int32_t serial) override;

  private:
    sp<V1_0::IRadio> mRealRadio;
    sp<RadioResponse> mRadioResponse = new RadioResponse();
    sp<RadioIndication> mRadioIndication = new RadioIndication();

    sp<V1_1::IRadio> getRealRadio_V1_1();
    sp<V1_2::IRadio> getRealRadio_V1_2();
    sp<V1_3::IRadio> getRealRadio_V1_3();
    sp<V1_4::IRadio> getRealRadio_V1_4();
};

}  // namespace android::hardware::radio::implementation
