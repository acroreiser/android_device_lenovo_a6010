/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "RadioResponse.h"
#include "Helpers.h"
#include<string>

namespace android::hardware::radio::implementation {

// Methods from ::android::hardware::radio::V1_0::IRadioResponse follow.
Return<void> RadioResponse::getIccCardStatusResponse(const V1_0::RadioResponseInfo& info,
                                                     const V1_0::CardStatus& cardStatus) {
    V1_4::CardStatus newCS = {};
    newCS.base.base = cardStatus;
    newCS.base.physicalSlotId = -1;
    newCS.base.iccid = hidl_string(" ");
    return mRealRadioResponse->getIccCardStatusResponse_1_4(info, newCS);
}

Return<void> RadioResponse::supplyIccPinForAppResponse(const V1_0::RadioResponseInfo& info,
                                                       int32_t remainingRetries) {
    return mRealRadioResponse->supplyIccPinForAppResponse(info, remainingRetries);
}

Return<void> RadioResponse::supplyIccPukForAppResponse(const V1_0::RadioResponseInfo& info,
                                                       int32_t remainingRetries) {
    return mRealRadioResponse->supplyIccPukForAppResponse(info, remainingRetries);
}

Return<void> RadioResponse::supplyIccPin2ForAppResponse(const V1_0::RadioResponseInfo& info,
                                                        int32_t remainingRetries) {
    return mRealRadioResponse->supplyIccPin2ForAppResponse(info, remainingRetries);
}

Return<void> RadioResponse::supplyIccPuk2ForAppResponse(const V1_0::RadioResponseInfo& info,
                                                        int32_t remainingRetries) {
    return mRealRadioResponse->supplyIccPuk2ForAppResponse(info, remainingRetries);
}

Return<void> RadioResponse::changeIccPinForAppResponse(const V1_0::RadioResponseInfo& info,
                                                       int32_t remainingRetries) {
    return mRealRadioResponse->changeIccPinForAppResponse(info, remainingRetries);
}

Return<void> RadioResponse::changeIccPin2ForAppResponse(const V1_0::RadioResponseInfo& info,
                                                        int32_t remainingRetries) {
    return mRealRadioResponse->changeIccPin2ForAppResponse(info, remainingRetries);
}

Return<void> RadioResponse::supplyNetworkDepersonalizationResponse(
        const V1_0::RadioResponseInfo& info, int32_t remainingRetries) {
    return mRealRadioResponse->supplyNetworkDepersonalizationResponse(info, remainingRetries);
}

Return<void> RadioResponse::getCurrentCallsResponse(const V1_0::RadioResponseInfo& info,
                                                    const hidl_vec<V1_0::Call>& calls) {
    hidl_vec<V1_2::Call> newCalls;
    newCalls.resize(calls.size());

    for(int x = 0; x < calls.size(); ++x){
        newCalls[x].base = calls[x];
        newCalls[x].audioQuality = V1_2::AudioQuality::UNSPECIFIED;
    }
    return mRealRadioResponse->getCurrentCallsResponse_1_2(info, newCalls);
}

Return<void> RadioResponse::dialResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->dialResponse(info);
}

Return<void> RadioResponse::getIMSIForAppResponse(const V1_0::RadioResponseInfo& info,
                                                  const hidl_string& imsi) {
    return mRealRadioResponse->getIMSIForAppResponse(info, imsi);
}

Return<void> RadioResponse::hangupConnectionResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->hangupConnectionResponse(info);
}

Return<void> RadioResponse::hangupWaitingOrBackgroundResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->hangupWaitingOrBackgroundResponse(info);
}

Return<void> RadioResponse::hangupForegroundResumeBackgroundResponse(
        const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->hangupForegroundResumeBackgroundResponse(info);
}

Return<void> RadioResponse::switchWaitingOrHoldingAndActiveResponse(
        const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->switchWaitingOrHoldingAndActiveResponse(info);
}

Return<void> RadioResponse::conferenceResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->conferenceResponse(info);
}

Return<void> RadioResponse::rejectCallResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->rejectCallResponse(info);
}

Return<void> RadioResponse::getLastCallFailCauseResponse(
        const V1_0::RadioResponseInfo& info, const V1_0::LastCallFailCauseInfo& failCauseinfo) {
    return mRealRadioResponse->getLastCallFailCauseResponse(info, failCauseinfo);
}

Return<void> RadioResponse::getSignalStrengthResponse(const V1_0::RadioResponseInfo& info,
                                                      const V1_0::SignalStrength& sigStrength) {
    return mRealRadioResponse->getSignalStrengthResponse_1_4(info, Create1_4SignalStrength(sigStrength));
}

void Init1_2CellIdentity(const V1_0::CellIdentity& legacyCI, V1_2::CellIdentity& newCI) {
    if(legacyCI.cellIdentityGsm.size() == 1){
        newCI.cellIdentityGsm.resize(1);
        newCI.cellIdentityGsm[0].base = legacyCI.cellIdentityGsm[0];
    }
    else if(legacyCI.cellIdentityCdma.size() == 1){
        newCI.cellIdentityCdma.resize(1);
        newCI.cellIdentityCdma[0].base = legacyCI.cellIdentityCdma[0];
    }
    else if(legacyCI.cellIdentityLte.size() == 1){
        newCI.cellIdentityLte.resize(1);
        newCI.cellIdentityLte[0].base = legacyCI.cellIdentityLte[0];
        newCI.cellIdentityLte[0].bandwidth = INT_MAX;
    }
    else if(legacyCI.cellIdentityWcdma.size() == 1){
        newCI.cellIdentityWcdma.resize(1);
        newCI.cellIdentityWcdma[0].base = legacyCI.cellIdentityWcdma[0];
    }
    else if(legacyCI.cellIdentityTdscdma.size() == 1){
        newCI.cellIdentityTdscdma.resize(1);
        newCI.cellIdentityTdscdma[0].base = legacyCI.cellIdentityTdscdma[0];
        newCI.cellIdentityTdscdma[0].uarfcn = INT_MAX;
    }
}

Return<void> RadioResponse::getVoiceRegistrationStateResponse(
        const V1_0::RadioResponseInfo& info, const V1_0::VoiceRegStateResult& voiceRegResponse) {
    V1_2::VoiceRegStateResult newVRR = {};
    newVRR.regState = voiceRegResponse.regState;
    newVRR.rat = voiceRegResponse.rat;
    newVRR.cssSupported  = voiceRegResponse.cssSupported;
    newVRR.roamingIndicator = voiceRegResponse.roamingIndicator;
    newVRR.systemIsInPrl = voiceRegResponse.systemIsInPrl;
    newVRR.defaultRoamingIndicator = voiceRegResponse.defaultRoamingIndicator;
    newVRR.reasonForDenial = voiceRegResponse.reasonForDenial;
    Init1_2CellIdentity(voiceRegResponse.cellIdentity, newVRR.cellIdentity);

    return mRealRadioResponse->getVoiceRegistrationStateResponse_1_2(info, newVRR);
}

Return<void> RadioResponse::getDataRegistrationStateResponse(
        const V1_0::RadioResponseInfo& info, const V1_0::DataRegStateResult& dataRegResponse) {
    mDataRoaming = (dataRegResponse.regState == V1_0::RegState::REG_ROAMING);
    mRat = (V1_0::RadioTechnology) dataRegResponse.rat;

    V1_4::DataRegStateResult newDRR = {};
    newDRR.base.regState = dataRegResponse.regState;
    newDRR.base.rat = dataRegResponse.rat;
    newDRR.base.reasonDataDenied = dataRegResponse.reasonDataDenied;
    newDRR.base.maxDataCalls = dataRegResponse.maxDataCalls;
    Init1_2CellIdentity(dataRegResponse.cellIdentity, newDRR.base.cellIdentity);

    return mRealRadioResponse->getDataRegistrationStateResponse_1_4(info, newDRR);
}

Return<void> RadioResponse::getOperatorResponse(const V1_0::RadioResponseInfo& info,
                                                const hidl_string& longName,
                                                const hidl_string& shortName,
                                                const hidl_string& numeric) {
    return mRealRadioResponse->getOperatorResponse(info, longName, shortName, numeric);
}

Return<void> RadioResponse::setRadioPowerResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setRadioPowerResponse(info);
}

Return<void> RadioResponse::sendDtmfResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->sendDtmfResponse(info);
}

Return<void> RadioResponse::sendSmsResponse(const V1_0::RadioResponseInfo& info,
                                            const V1_0::SendSmsResult& sms) {
    return mRealRadioResponse->sendSmsResponse(info, sms);
}

Return<void> RadioResponse::sendSMSExpectMoreResponse(const V1_0::RadioResponseInfo& info,
                                                      const V1_0::SendSmsResult& sms) {
    return mRealRadioResponse->sendSMSExpectMoreResponse(info, sms);
}

Return<void> RadioResponse::setupDataCallResponse(const V1_0::RadioResponseInfo& info,
                                                  const V1_0::SetupDataCallResult& dcResponse) {
    return mRealRadioResponse->setupDataCallResponse_1_4(info, Create1_4SetupDataCallResult(dcResponse));
}

Return<void> RadioResponse::iccIOForAppResponse(const V1_0::RadioResponseInfo& info,
                                                const V1_0::IccIoResult& iccIo) {
    return mRealRadioResponse->iccIOForAppResponse(info, iccIo);
}

Return<void> RadioResponse::sendUssdResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->sendUssdResponse(info);
}

Return<void> RadioResponse::cancelPendingUssdResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->cancelPendingUssdResponse(info);
}

Return<void> RadioResponse::getClirResponse(const V1_0::RadioResponseInfo& info, int32_t n,
                                            int32_t m) {
    return mRealRadioResponse->getClirResponse(info, n, m);
}

Return<void> RadioResponse::setClirResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setClirResponse(info);
}

Return<void> RadioResponse::getCallForwardStatusResponse(
        const V1_0::RadioResponseInfo& info,
        const hidl_vec<V1_0::CallForwardInfo>& callForwardInfos) {
    return mRealRadioResponse->getCallForwardStatusResponse(info, callForwardInfos);
}

Return<void> RadioResponse::setCallForwardResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setCallForwardResponse(info);
}

Return<void> RadioResponse::getCallWaitingResponse(const V1_0::RadioResponseInfo& info, bool enable,
                                                   int32_t serviceClass) {
    return mRealRadioResponse->getCallWaitingResponse(info, enable, serviceClass);
}

Return<void> RadioResponse::setCallWaitingResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setCallWaitingResponse(info);
}

Return<void> RadioResponse::acknowledgeLastIncomingGsmSmsResponse(
        const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->acknowledgeLastIncomingGsmSmsResponse(info);
}

Return<void> RadioResponse::acceptCallResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->acceptCallResponse(info);
}

Return<void> RadioResponse::deactivateDataCallResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->deactivateDataCallResponse(info);
}

Return<void> RadioResponse::getFacilityLockForAppResponse(const V1_0::RadioResponseInfo& info,
                                                          int32_t response) {
    return mRealRadioResponse->getFacilityLockForAppResponse(info, response);
}

Return<void> RadioResponse::setFacilityLockForAppResponse(const V1_0::RadioResponseInfo& info,
                                                          int32_t retry) {
    return mRealRadioResponse->setFacilityLockForAppResponse(info, retry);
}

Return<void> RadioResponse::setBarringPasswordResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setBarringPasswordResponse(info);
}

Return<void> RadioResponse::getNetworkSelectionModeResponse(const V1_0::RadioResponseInfo& info,
                                                            bool manual) {
    return mRealRadioResponse->getNetworkSelectionModeResponse(info, manual);
}

Return<void> RadioResponse::setNetworkSelectionModeAutomaticResponse(
        const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setNetworkSelectionModeAutomaticResponse(info);
}

Return<void> RadioResponse::setNetworkSelectionModeManualResponse(
        const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setNetworkSelectionModeManualResponse(info);
}

Return<void> RadioResponse::getAvailableNetworksResponse(
        const V1_0::RadioResponseInfo& info, const hidl_vec<V1_0::OperatorInfo>& networkInfos) {
    return mRealRadioResponse->getAvailableNetworksResponse(info, networkInfos);
}

Return<void> RadioResponse::startDtmfResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->startDtmfResponse(info);
}

Return<void> RadioResponse::stopDtmfResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->stopDtmfResponse(info);
}

Return<void> RadioResponse::getBasebandVersionResponse(const V1_0::RadioResponseInfo& info,
                                                       const hidl_string& version) {
    return mRealRadioResponse->getBasebandVersionResponse(info, version);
}

Return<void> RadioResponse::separateConnectionResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->separateConnectionResponse(info);
}

Return<void> RadioResponse::setMuteResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setMuteResponse(info);
}

Return<void> RadioResponse::getMuteResponse(const V1_0::RadioResponseInfo& info, bool enable) {
    return mRealRadioResponse->getMuteResponse(info, enable);
}

Return<void> RadioResponse::getClipResponse(const V1_0::RadioResponseInfo& info,
                                            V1_0::ClipStatus status) {
    return mRealRadioResponse->getClipResponse(info, status);
}

Return<void> RadioResponse::getDataCallListResponse(
        const V1_0::RadioResponseInfo& info,
        const hidl_vec<V1_0::SetupDataCallResult>& dcResponse) {
    hidl_vec<V1_4::SetupDataCallResult> newResponse;
    newResponse.resize(dcResponse.size());

    for(int x = 0; x < dcResponse.size(); ++x)
        newResponse[x] = Create1_4SetupDataCallResult(dcResponse[x]);

    return mRealRadioResponse->getDataCallListResponse_1_4(info, newResponse);
}

Return<void> RadioResponse::setSuppServiceNotificationsResponse(
        const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setSuppServiceNotificationsResponse(info);
}

Return<void> RadioResponse::writeSmsToSimResponse(const V1_0::RadioResponseInfo& info,
                                                  int32_t index) {
    return mRealRadioResponse->writeSmsToSimResponse(info, index);
}

Return<void> RadioResponse::deleteSmsOnSimResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->deleteSmsOnSimResponse(info);
}

Return<void> RadioResponse::setBandModeResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setBandModeResponse(info);
}

Return<void> RadioResponse::getAvailableBandModesResponse(
        const V1_0::RadioResponseInfo& info, const hidl_vec<V1_0::RadioBandMode>& bandModes) {
    return mRealRadioResponse->getAvailableBandModesResponse(info, bandModes);
}

Return<void> RadioResponse::sendEnvelopeResponse(const V1_0::RadioResponseInfo& info,
                                                 const hidl_string& commandResponse) {
    return mRealRadioResponse->sendEnvelopeResponse(info, commandResponse);
}

Return<void> RadioResponse::sendTerminalResponseToSimResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->sendTerminalResponseToSimResponse(info);
}

Return<void> RadioResponse::handleStkCallSetupRequestFromSimResponse(
        const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->handleStkCallSetupRequestFromSimResponse(info);
}

Return<void> RadioResponse::explicitCallTransferResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->explicitCallTransferResponse(info);
}

Return<void> RadioResponse::setPreferredNetworkTypeResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setPreferredNetworkTypeBitmapResponse(info);
}

Return<void> RadioResponse::getPreferredNetworkTypeResponse(const V1_0::RadioResponseInfo& info,
                                                            V1_0::PreferredNetworkType nwType) {
    hidl_bitfield<V1_4::RadioAccessFamily> nwTypeBitmap = 0;
    switch(nwType){
        case V1_0::PreferredNetworkType::GSM_WCDMA:
        case V1_0::PreferredNetworkType::GSM_WCDMA_AUTO:
            nwTypeBitmap = GSMBITS | WCDMABITS;
        break;
        case V1_0::PreferredNetworkType::GSM_ONLY:
            nwTypeBitmap = GSMBITS;
        break;
        case V1_0::PreferredNetworkType::WCDMA:
            nwTypeBitmap = WCDMABITS;
        break;
        case V1_0::PreferredNetworkType::CDMA_EVDO_AUTO:
            nwTypeBitmap = CDMABITS | EVDOBITS;
        break;
        case V1_0::PreferredNetworkType::CDMA_ONLY:
            nwTypeBitmap = CDMABITS;
        break;
        case V1_0::PreferredNetworkType::EVDO_ONLY:
            nwTypeBitmap = EVDOBITS;
        break;
        case V1_0::PreferredNetworkType::GSM_WCDMA_CDMA_EVDO_AUTO:
            nwTypeBitmap = GSMBITS | WCDMABITS | CDMABITS | EVDOBITS;
        break;
        case V1_0::PreferredNetworkType::LTE_CDMA_EVDO:
            nwTypeBitmap = LTEBITS | CDMABITS | EVDOBITS;
        break;
        case V1_0::PreferredNetworkType::LTE_GSM_WCDMA:
            nwTypeBitmap = LTEBITS | GSMBITS | WCDMABITS;
        break;
        case V1_0::PreferredNetworkType::LTE_CMDA_EVDO_GSM_WCDMA:
            nwTypeBitmap = LTEBITS | CDMABITS | EVDOBITS | GSMBITS | WCDMABITS;
        break;
        case V1_0::PreferredNetworkType::LTE_ONLY:
            nwTypeBitmap = LTEBITS;
        break;
        case V1_0::PreferredNetworkType::LTE_WCDMA:
            nwTypeBitmap = LTEBITS | WCDMABITS;
        break;
        case V1_0::PreferredNetworkType::TD_SCDMA_ONLY:
            nwTypeBitmap = TDSCDMABIT;
        break;
        case V1_0::PreferredNetworkType::TD_SCDMA_WCDMA:
            nwTypeBitmap = TDSCDMABIT | WCDMABITS;
        break;
        case V1_0::PreferredNetworkType::TD_SCDMA_LTE:
            nwTypeBitmap = TDSCDMABIT | LTEBITS;
        break;
        case V1_0::PreferredNetworkType::TD_SCDMA_GSM:
            nwTypeBitmap = TDSCDMABIT | GSMBITS;
        break;
        case V1_0::PreferredNetworkType::TD_SCDMA_GSM_LTE:
            nwTypeBitmap = TDSCDMABIT | GSMBITS | LTEBITS;
        break;
        case V1_0::PreferredNetworkType::TD_SCDMA_GSM_WCDMA:
            nwTypeBitmap = TDSCDMABIT | GSMBITS | WCDMABITS;
        break;
        case V1_0::PreferredNetworkType::TD_SCDMA_WCDMA_LTE:
            nwTypeBitmap = TDSCDMABIT | WCDMABITS | LTEBITS;
        break;
        case V1_0::PreferredNetworkType::TD_SCDMA_GSM_WCDMA_LTE:
            nwTypeBitmap = TDSCDMABIT | GSMBITS | WCDMABITS | LTEBITS;
        break;
        case V1_0::PreferredNetworkType::TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO:
            nwTypeBitmap = TDSCDMABIT | GSMBITS | WCDMABITS | CDMABITS | EVDOBITS;
        break;
        case V1_0::PreferredNetworkType::TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA:
            nwTypeBitmap = TDSCDMABIT | LTEBITS | CDMABITS | EVDOBITS | GSMBITS | WCDMABITS;
        break;
    }
    return mRealRadioResponse->getPreferredNetworkTypeBitmapResponse(info, nwTypeBitmap);
}

Return<void> RadioResponse::getNeighboringCidsResponse(
        const V1_0::RadioResponseInfo& info, const hidl_vec<V1_0::NeighboringCell>& cells) {
    return mRealRadioResponse->getNeighboringCidsResponse(info, cells);
}

Return<void> RadioResponse::setLocationUpdatesResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setLocationUpdatesResponse(info);
}

Return<void> RadioResponse::setCdmaSubscriptionSourceResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setCdmaSubscriptionSourceResponse(info);
}

Return<void> RadioResponse::setCdmaRoamingPreferenceResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setCdmaRoamingPreferenceResponse(info);
}

Return<void> RadioResponse::getCdmaRoamingPreferenceResponse(const V1_0::RadioResponseInfo& info,
                                                             V1_0::CdmaRoamingType type) {
    return mRealRadioResponse->getCdmaRoamingPreferenceResponse(info, type);
}

Return<void> RadioResponse::setTTYModeResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setTTYModeResponse(info);
}

Return<void> RadioResponse::getTTYModeResponse(const V1_0::RadioResponseInfo& info,
                                               V1_0::TtyMode mode) {
    return mRealRadioResponse->getTTYModeResponse(info, mode);
}

Return<void> RadioResponse::setPreferredVoicePrivacyResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setPreferredVoicePrivacyResponse(info);
}

Return<void> RadioResponse::getPreferredVoicePrivacyResponse(const V1_0::RadioResponseInfo& info,
                                                             bool enable) {
    return mRealRadioResponse->getPreferredVoicePrivacyResponse(info, enable);
}

Return<void> RadioResponse::sendCDMAFeatureCodeResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->sendCDMAFeatureCodeResponse(info);
}

Return<void> RadioResponse::sendBurstDtmfResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->sendBurstDtmfResponse(info);
}

Return<void> RadioResponse::sendCdmaSmsResponse(const V1_0::RadioResponseInfo& info,
                                                const V1_0::SendSmsResult& sms) {
    return mRealRadioResponse->sendCdmaSmsResponse(info, sms);
}

Return<void> RadioResponse::acknowledgeLastIncomingCdmaSmsResponse(
        const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->acknowledgeLastIncomingCdmaSmsResponse(info);
}

Return<void> RadioResponse::getGsmBroadcastConfigResponse(
        const V1_0::RadioResponseInfo& info,
        const hidl_vec<V1_0::GsmBroadcastSmsConfigInfo>& configs) {
    return mRealRadioResponse->getGsmBroadcastConfigResponse(info, configs);
}

Return<void> RadioResponse::setGsmBroadcastConfigResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setGsmBroadcastConfigResponse(info);
}

Return<void> RadioResponse::setGsmBroadcastActivationResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setGsmBroadcastActivationResponse(info);
}

Return<void> RadioResponse::getCdmaBroadcastConfigResponse(
        const V1_0::RadioResponseInfo& info,
        const hidl_vec<V1_0::CdmaBroadcastSmsConfigInfo>& configs) {
    return mRealRadioResponse->getCdmaBroadcastConfigResponse(info, configs);
}

Return<void> RadioResponse::setCdmaBroadcastConfigResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setCdmaBroadcastConfigResponse(info);
}

Return<void> RadioResponse::setCdmaBroadcastActivationResponse(
        const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setCdmaBroadcastActivationResponse(info);
}

Return<void> RadioResponse::getCDMASubscriptionResponse(
        const V1_0::RadioResponseInfo& info, const hidl_string& mdn, const hidl_string& hSid,
        const hidl_string& hNid, const hidl_string& min, const hidl_string& prl) {
    return mRealRadioResponse->getCDMASubscriptionResponse(info, mdn, hSid, hNid, min, prl);
}

Return<void> RadioResponse::writeSmsToRuimResponse(const V1_0::RadioResponseInfo& info,
                                                   uint32_t index) {
    return mRealRadioResponse->writeSmsToRuimResponse(info, index);
}

Return<void> RadioResponse::deleteSmsOnRuimResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->deleteSmsOnRuimResponse(info);
}

Return<void> RadioResponse::getDeviceIdentityResponse(const V1_0::RadioResponseInfo& info,
                                                      const hidl_string& imei,
                                                      const hidl_string& imeisv,
                                                      const hidl_string& esn,
                                                      const hidl_string& meid) {
    return mRealRadioResponse->getDeviceIdentityResponse(info, imei, imeisv, esn, meid);
}

Return<void> RadioResponse::exitEmergencyCallbackModeResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->exitEmergencyCallbackModeResponse(info);
}

Return<void> RadioResponse::getSmscAddressResponse(const V1_0::RadioResponseInfo& info,
                                                   const hidl_string& smsc) {
    return mRealRadioResponse->getSmscAddressResponse(info, smsc);
}

Return<void> RadioResponse::setSmscAddressResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setSmscAddressResponse(info);
}

Return<void> RadioResponse::reportSmsMemoryStatusResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->reportSmsMemoryStatusResponse(info);
}

Return<void> RadioResponse::reportStkServiceIsRunningResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->reportStkServiceIsRunningResponse(info);
}

Return<void> RadioResponse::getCdmaSubscriptionSourceResponse(const V1_0::RadioResponseInfo& info,
                                                              V1_0::CdmaSubscriptionSource source) {
    return mRealRadioResponse->getCdmaSubscriptionSourceResponse(info, source);
}

Return<void> RadioResponse::requestIsimAuthenticationResponse(const V1_0::RadioResponseInfo& info,
                                                              const hidl_string& response) {
    return mRealRadioResponse->requestIsimAuthenticationResponse(info, response);
}

Return<void> RadioResponse::acknowledgeIncomingGsmSmsWithPduResponse(
        const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->acknowledgeIncomingGsmSmsWithPduResponse(info);
}

Return<void> RadioResponse::sendEnvelopeWithStatusResponse(const V1_0::RadioResponseInfo& info,
                                                           const V1_0::IccIoResult& iccIo) {
    return mRealRadioResponse->sendEnvelopeWithStatusResponse(info, iccIo);
}

Return<void> RadioResponse::getVoiceRadioTechnologyResponse(const V1_0::RadioResponseInfo& info,
                                                            V1_0::RadioTechnology rat) {
    return mRealRadioResponse->getVoiceRadioTechnologyResponse(info, rat);
}

Return<void> RadioResponse::getCellInfoListResponse(const V1_0::RadioResponseInfo& info,
                                                    const hidl_vec<V1_0::CellInfo>& cellInfo) {
    return mRealRadioResponse->getCellInfoListResponse_1_4(info, Create1_4CellInfoList(cellInfo));
}

Return<void> RadioResponse::setCellInfoListRateResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setCellInfoListRateResponse(info);
}

Return<void> RadioResponse::setInitialAttachApnResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setInitialAttachApnResponse(info);
}

Return<void> RadioResponse::getImsRegistrationStateResponse(const V1_0::RadioResponseInfo& info,
                                                            bool isRegistered,
                                                            V1_0::RadioTechnologyFamily ratFamily) {
    return mRealRadioResponse->getImsRegistrationStateResponse(info, isRegistered, ratFamily);
}

Return<void> RadioResponse::sendImsSmsResponse(const V1_0::RadioResponseInfo& info,
                                               const V1_0::SendSmsResult& sms) {
    return mRealRadioResponse->sendImsSmsResponse(info, sms);
}

Return<void> RadioResponse::iccTransmitApduBasicChannelResponse(const V1_0::RadioResponseInfo& info,
                                                                const V1_0::IccIoResult& result) {
    return mRealRadioResponse->iccTransmitApduBasicChannelResponse(info, result);
}

Return<void> RadioResponse::iccOpenLogicalChannelResponse(const V1_0::RadioResponseInfo& info,
                                                          int32_t channelId,
                                                          const hidl_vec<int8_t>& selectResponse) {
    return mRealRadioResponse->iccOpenLogicalChannelResponse(info, channelId, selectResponse);
}

Return<void> RadioResponse::iccCloseLogicalChannelResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->iccCloseLogicalChannelResponse(info);
}

Return<void> RadioResponse::iccTransmitApduLogicalChannelResponse(
        const V1_0::RadioResponseInfo& info, const V1_0::IccIoResult& result) {
    return mRealRadioResponse->iccTransmitApduLogicalChannelResponse(info, result);
}

Return<void> RadioResponse::nvReadItemResponse(const V1_0::RadioResponseInfo& info,
                                               const hidl_string& result) {
    return mRealRadioResponse->nvReadItemResponse(info, result);
}

Return<void> RadioResponse::nvWriteItemResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->nvWriteItemResponse(info);
}

Return<void> RadioResponse::nvWriteCdmaPrlResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->nvWriteCdmaPrlResponse(info);
}

Return<void> RadioResponse::nvResetConfigResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->nvResetConfigResponse(info);
}

Return<void> RadioResponse::setUiccSubscriptionResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setUiccSubscriptionResponse(info);
}

Return<void> RadioResponse::setDataAllowedResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setDataAllowedResponse(info);
}

Return<void> RadioResponse::getHardwareConfigResponse(
        const V1_0::RadioResponseInfo& info, const hidl_vec<V1_0::HardwareConfig>& config) {
    return mRealRadioResponse->getHardwareConfigResponse(info, config);
}

Return<void> RadioResponse::requestIccSimAuthenticationResponse(const V1_0::RadioResponseInfo& info,
                                                                const V1_0::IccIoResult& result) {
    return mRealRadioResponse->requestIccSimAuthenticationResponse(info, result);
}

Return<void> RadioResponse::setDataProfileResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setDataProfileResponse(info);
}

Return<void> RadioResponse::requestShutdownResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->requestShutdownResponse(info);
}

Return<void> RadioResponse::getRadioCapabilityResponse(const V1_0::RadioResponseInfo& info,
                                                       const V1_0::RadioCapability& rc) {
    return mRealRadioResponse->getRadioCapabilityResponse(info, rc);
}

Return<void> RadioResponse::setRadioCapabilityResponse(const V1_0::RadioResponseInfo& info,
                                                       const V1_0::RadioCapability& rc) {
    return mRealRadioResponse->setRadioCapabilityResponse(info, rc);
}

Return<void> RadioResponse::startLceServiceResponse(const V1_0::RadioResponseInfo& info,
                                                    const V1_0::LceStatusInfo& statusInfo) {
    return mRealRadioResponse->startLceServiceResponse(info, statusInfo);
}

Return<void> RadioResponse::stopLceServiceResponse(const V1_0::RadioResponseInfo& info,
                                                   const V1_0::LceStatusInfo& statusInfo) {
    return mRealRadioResponse->stopLceServiceResponse(info, statusInfo);
}

Return<void> RadioResponse::pullLceDataResponse(const V1_0::RadioResponseInfo& info,
                                                const V1_0::LceDataInfo& lceInfo) {
    return mRealRadioResponse->pullLceDataResponse(info, lceInfo);
}

Return<void> RadioResponse::getModemActivityInfoResponse(
        const V1_0::RadioResponseInfo& info, const V1_0::ActivityStatsInfo& activityInfo) {
    return mRealRadioResponse->getModemActivityInfoResponse(info, activityInfo);
}

Return<void> RadioResponse::setAllowedCarriersResponse(const V1_0::RadioResponseInfo& info,
                                                       int32_t /* numAllowed */) {
    return mRealRadioResponse->setAllowedCarriersResponse_1_4(info);
}

Return<void> RadioResponse::getAllowedCarriersResponse(const V1_0::RadioResponseInfo& info,
                                                       bool allAllowed,
                                                       const V1_0::CarrierRestrictions& carriers) {
    V1_4::CarrierRestrictionsWithPriority newCarriers = {};
    if(allAllowed){
        newCarriers.allowedCarriersPrioritized = false;
        return mRealRadioResponse->getAllowedCarriersResponse_1_4(info, newCarriers, V1_4::SimLockMultiSimPolicy::NO_MULTISIM_POLICY);
    }

    newCarriers.allowedCarriers = carriers.allowedCarriers;
    newCarriers.excludedCarriers = carriers.excludedCarriers;
    newCarriers.allowedCarriersPrioritized = true;
    return mRealRadioResponse->getAllowedCarriersResponse_1_4(info, newCarriers, V1_4::SimLockMultiSimPolicy::NO_MULTISIM_POLICY);
}

Return<void> RadioResponse::sendDeviceStateResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->sendDeviceStateResponse(info);
}

Return<void> RadioResponse::setIndicationFilterResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setIndicationFilterResponse(info);
}

Return<void> RadioResponse::setSimCardPowerResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setSimCardPowerResponse_1_1(info);
}

Return<void> RadioResponse::acknowledgeRequest(int32_t serial) {
    return mRealRadioResponse->acknowledgeRequest(serial);
}

// Methods from ::android::hardware::radio::V1_1::IRadioResponse follow.
Return<void> RadioResponse::setCarrierInfoForImsiEncryptionResponse(
        const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setCarrierInfoForImsiEncryptionResponse(info);
}

Return<void> RadioResponse::setSimCardPowerResponse_1_1(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setSimCardPowerResponse_1_1(info);
}

Return<void> RadioResponse::startNetworkScanResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->startNetworkScanResponse_1_4(info);
}


Return<void> RadioResponse::stopNetworkScanResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->stopNetworkScanResponse(info);
}

Return<void> RadioResponse::startKeepaliveResponse(const V1_0::RadioResponseInfo& info,
                                                   const V1_1::KeepaliveStatus& status) {
    return mRealRadioResponse->startKeepaliveResponse(info, status);
}

Return<void> RadioResponse::stopKeepaliveResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->stopKeepaliveResponse(info);
}

// Methods from ::android::hardware::radio::V1_2::IRadioResponse follow.
Return<void> RadioResponse::getCellInfoListResponse_1_2(const V1_0::RadioResponseInfo& info,
                                                        const hidl_vec<V1_2::CellInfo>& cellInfo) {
    return mRealRadioResponse->getCellInfoListResponse_1_4(info, Create1_4CellInfoList(cellInfo));
}

Return<void> RadioResponse::getIccCardStatusResponse_1_2(const V1_0::RadioResponseInfo& info,
                                                         const V1_2::CardStatus& cardStatus) {
    return mRealRadioResponse->getIccCardStatusResponse_1_4(info, {cardStatus, hidl_string("")});
}

Return<void> RadioResponse::setSignalStrengthReportingCriteriaResponse(
        const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setSignalStrengthReportingCriteriaResponse(info);
}

Return<void> RadioResponse::setLinkCapacityReportingCriteriaResponse(
        const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setLinkCapacityReportingCriteriaResponse(info);
}

Return<void> RadioResponse::getCurrentCallsResponse_1_2(const V1_0::RadioResponseInfo& info,
                                                        const hidl_vec<V1_2::Call>& calls) {
    return mRealRadioResponse->getCurrentCallsResponse_1_2(info, calls);
}

Return<void> RadioResponse::getSignalStrengthResponse_1_2(
        const V1_0::RadioResponseInfo& info, const V1_2::SignalStrength& signalStrength) {
    return mRealRadioResponse->getSignalStrengthResponse_1_4(info, Create1_4SignalStrength(signalStrength));
}

Return<void> RadioResponse::getVoiceRegistrationStateResponse_1_2(
        const V1_0::RadioResponseInfo& info, const V1_2::VoiceRegStateResult& voiceRegResponse) {
    return mRealRadioResponse->getVoiceRegistrationStateResponse_1_2(info, voiceRegResponse);
}

Return<void> RadioResponse::getDataRegistrationStateResponse_1_2(
        const V1_0::RadioResponseInfo& info, const V1_2::DataRegStateResult& dataRegResponse) {
    mDataRoaming = (dataRegResponse.regState == V1_0::RegState::REG_ROAMING);
    V1_4::DataRegStateResult newDRR = {};
    newDRR.base = dataRegResponse;
    return mRealRadioResponse->getDataRegistrationStateResponse_1_4(info, newDRR);
}

// Methods from ::android::hardware::radio::V1_3::IRadioResponse follow.
Return<void> RadioResponse::setSystemSelectionChannelsResponse(
        const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setSystemSelectionChannelsResponse(info);
}

Return<void> RadioResponse::enableModemResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->enableModemResponse(info);
}

Return<void> RadioResponse::getModemStackStatusResponse(const V1_0::RadioResponseInfo& info,
                                                        bool isEnabled) {
    return mRealRadioResponse->getModemStackStatusResponse(info, isEnabled);
}

// Methods from ::android::hardware::radio::V1_4::IRadioResponse follow.
Return<void> RadioResponse::emergencyDialResponse(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->emergencyDialResponse(info);
}

Return<void> RadioResponse::startNetworkScanResponse_1_4(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->startNetworkScanResponse_1_4(info);
}

Return<void> RadioResponse::getCellInfoListResponse_1_4(const V1_0::RadioResponseInfo& info,
                                                        const hidl_vec<V1_4::CellInfo>& cellInfo) {
    return mRealRadioResponse->getCellInfoListResponse_1_4(info, cellInfo);
}

Return<void> RadioResponse::getDataRegistrationStateResponse_1_4(
        const V1_0::RadioResponseInfo& info, const V1_4::DataRegStateResult& dataRegResponse) {
    return mRealRadioResponse->getDataRegistrationStateResponse_1_4(info, dataRegResponse);
}

Return<void> RadioResponse::getIccCardStatusResponse_1_4(const V1_0::RadioResponseInfo& info,
                                                         const V1_4::CardStatus& cardStatus) {
    return mRealRadioResponse->getIccCardStatusResponse_1_4(info, cardStatus);
}

Return<void> RadioResponse::getPreferredNetworkTypeBitmapResponse(
        const V1_0::RadioResponseInfo& info, hidl_bitfield<V1_4::RadioAccessFamily> networkTypeBitmap) {
    return mRealRadioResponse->getPreferredNetworkTypeBitmapResponse(info, networkTypeBitmap);
}

Return<void> RadioResponse::setPreferredNetworkTypeBitmapResponse(
        const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setPreferredNetworkTypeBitmapResponse(info);
}

Return<void> RadioResponse::getDataCallListResponse_1_4(
        const V1_0::RadioResponseInfo& info,
        const hidl_vec<V1_4::SetupDataCallResult>& dcResponse) {
    return mRealRadioResponse->getDataCallListResponse_1_4(info, dcResponse);
}

Return<void> RadioResponse::setupDataCallResponse_1_4(const V1_0::RadioResponseInfo& info,
                                                      const V1_4::SetupDataCallResult& dcResponse) {
    return mRealRadioResponse->setupDataCallResponse_1_4(info, dcResponse);
}

Return<void> RadioResponse::setAllowedCarriersResponse_1_4(const V1_0::RadioResponseInfo& info) {
    return mRealRadioResponse->setAllowedCarriersResponse_1_4(info);
}

Return<void> RadioResponse::getAllowedCarriersResponse_1_4(
        const V1_0::RadioResponseInfo& info, const V1_4::CarrierRestrictionsWithPriority& carriers,
        V1_4::SimLockMultiSimPolicy multiSimPolicy) {
    return mRealRadioResponse->getAllowedCarriersResponse_1_4(info, carriers, multiSimPolicy);
}

Return<void> RadioResponse::getSignalStrengthResponse_1_4(
        const V1_0::RadioResponseInfo& info, const V1_4::SignalStrength& signalStrength) {
    return mRealRadioResponse->getSignalStrengthResponse_1_4(info, signalStrength);
}

}  // namespace android::hardware::radio::implementation
