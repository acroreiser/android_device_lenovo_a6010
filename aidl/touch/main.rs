/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

//! This implements the Lineage KeyDisabler HAL.
mod keydisabler_hal;

use crate::keydisabler_hal::KeyDisablerHal;

use vendor_lineage_touch::aidl::vendor::lineage::touch::IKeyDisabler::{
    IKeyDisabler,
    BnKeyDisabler,
};

use log::LevelFilter;

const LOG_TAG: &str = "KeyDisablerHal";

fn main() {
    let logger_success = logger::init(
        logger::Config::default().with_tag_on_device(LOG_TAG).with_max_level(LevelFilter::Trace),
    );
    if !logger_success {
        panic!("{LOG_TAG}: Failed to start logger.");
    }

    binder::ProcessState::start_thread_pool();
    let my_service = KeyDisablerHal;
    let my_service_binder = BnKeyDisabler::new_binder(
        my_service,
        binder::BinderFeatures::default(),
    );
    let service_name = format!("{}/default", KeyDisablerHal::get_descriptor());
    binder::add_service(&service_name, my_service_binder.as_binder())
        .expect("Failed to register service");
    // Does not return.
    binder::ProcessState::join_thread_pool()
}
