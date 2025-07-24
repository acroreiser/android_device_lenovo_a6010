/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <android/hardware/radio/1.4/IRadioResponse.h>
#include <android/hardware/radio/1.4/IRadio.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;

#define GSMBITS (V1_0::RadioAccessFamily::GSM | V1_0::RadioAccessFamily::GPRS | V1_0::RadioAccessFamily::EDGE)
#define CDMABITS (V1_0::RadioAccessFamily::IS95A | V1_0::RadioAccessFamily::IS95B | V1_0::RadioAccessFamily::ONE_X_RTT)
#define EVDOBITS (V1_0::RadioAccessFamily::EVDO_0 | V1_0::RadioAccessFamily::EVDO_A | V1_0::RadioAccessFamily::EVDO_B | V1_0::RadioAccessFamily::EHRPD)
#define WCDMABITS (V1_0::RadioAccessFamily::HSUPA | V1_0::RadioAccessFamily::HSDPA | V1_0::RadioAccessFamily::HSPA | V1_0::RadioAccessFamily::HSPAP | V1_0::RadioAccessFamily::UMTS)
#define LTEBITS (V1_0::RadioAccessFamily::LTE | V1_0::RadioAccessFamily::LTE_CA)
#define TDSCDMABIT ((int) V1_0::RadioAccessFamily::TD_SCDMA)

android::hardware::radio::V1_4::SignalStrength Create1_4SignalStrength(const android::hardware::radio::V1_0::SignalStrength& sigStrength);
android::hardware::radio::V1_4::SignalStrength Create1_4SignalStrength(const android::hardware::radio::V1_2::SignalStrength& sigStrength);
hidl_vec<android::hardware::radio::V1_4::CellInfo> Create1_4CellInfoList(const hidl_vec<android::hardware::radio::V1_0::CellInfo>& cellInfo);
hidl_vec<android::hardware::radio::V1_4::CellInfo> Create1_4CellInfoList(const hidl_vec<android::hardware::radio::V1_2::CellInfo>& cellInfo);
android::hardware::radio::V1_4::SetupDataCallResult Create1_4SetupDataCallResult(const android::hardware::radio::V1_0::SetupDataCallResult& dcResponse);
