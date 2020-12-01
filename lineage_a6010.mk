# Copyright (C) 2015-2017 The CyanogenMod Project
# Copyright (C) 2017, The LineageOS Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/generic.mk)
$(call inherit-product, device/lenovo/a6010/device.mk)
$(call inherit-product, vendor/lineage/config/common_mini_go_phone.mk)

# Must define platform variant before including any common things
TARGET_BOARD_PLATFORM_VARIANT := msm8916

# Assert
TARGET_OTA_ASSERT_DEVICE := Kraft-T,a6000,K30-T,A6000,Kraft-W,Kraft-C,k30t,msm8916,Kraft-A6000,wt86518

TARGET_VENDOR := Lenovo
PRODUCT_DEVICE := a6010
PRODUCT_NAME := lineage_a6010
BOARD_VENDOR := Lenovo
PRODUCT_BRAND := Lenovo
PRODUCT_MODEL := A6010
PRODUCT_MANUFACTURER := Lenovo

# Device Identifiers
PRODUCT_BUILD_PROP_OVERRIDES += \
     PRODUCT_MODEL="Lenovo A6010" \
     PRODUCT_NAME="A6010" \
     PRODUCT_DEVICE="A6010"

PRODUCT_PROPERTY_OVERRIDES += \
     ro.build.product=A6010

# Build fingerprint
PRODUCT_BUILD_PROP_OVERRIDES += \
    PRIVATE_BUILD_DESC="A6010-user 5.0.2 LRX22G A6010_S045_170622_16G_ROW release-keys"

BUILD_FINGERPRINT := Lenovo/A6010/A6010:5.0.2/LRX22G/A6010_S045_170622_16G_ROW:user/release-keys

# GMS
PRODUCT_GMS_CLIENTID_BASE := android-lenovo

# Trust
PRODUCT_PACKAGES += \
    vendor.lineage.trust@1.0-service

# APN(s)
PRODUCT_COPY_FILES += vendor/lineage/prebuilt/common/etc/apns-conf.xml:system/etc/apns-conf.xml
