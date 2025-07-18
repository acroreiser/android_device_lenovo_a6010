/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

use binder::{Interface, Result as BinderResult, StatusCode};
use std::fs;

use vendor_lineage_touch::aidl::vendor::lineage::touch::{
    IKeyDisabler::IKeyDisabler,
};

const CONTROL_PATH: &str = "/sys/bus/i2c/drivers/ft5x06_ts/5-0038/disable_keys";
const CONTROL_PATH2: &str = "/sys/bus/i2c/drivers/mms200_i2c/5-0048/disable_keys";

pub struct KeyDisablerHal;

impl Interface for KeyDisablerHal {}

impl IKeyDisabler for KeyDisablerHal {
    fn getEnabled(&self) -> BinderResult<bool> {
        fs::read_to_string(CONTROL_PATH)
            .or_else(|err| {
                log::warn!("Failed to read from {}: {}", CONTROL_PATH, err);
                fs::read_to_string(CONTROL_PATH2)
            })
            .map(|mut s| {
                s.pop();
                s == "1"
            })
            .map_err(|err| {
                log::error!("Failed to read from {}: {}", CONTROL_PATH2, err);
                StatusCode::UNKNOWN_ERROR.into()
            })
    }

    fn setEnabled(&self, enabled: bool) -> BinderResult<()> {
        let value = if enabled { "1" } else { "0" };

        fs::write(CONTROL_PATH, value)
            .or_else(|err| {
                log::warn!("Failed to write to {}: {}", CONTROL_PATH, err);
                fs::write(CONTROL_PATH2, value)
            })
            .map_err(|err| {
                log::error!("Failed to write to {}: {}", CONTROL_PATH2, err);
                StatusCode::UNKNOWN_ERROR.into()
            })
    }
}
