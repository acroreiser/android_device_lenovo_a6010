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
TARGET_OTA_ASSERT_DEVICE := sisleyr

TARGET_VENDOR := Lenovo
PRODUCT_DEVICE := sisleyr
PRODUCT_NAME := lineage_sisleyr
BOARD_VENDOR := Lenovo
PRODUCT_BRAND := Lenovo
PRODUCT_MODEL := Lenovo S90-A
PRODUCT_MANUFACTURER := Lenovo

# Build fingerprint
PRODUCT_BUILD_PROP_OVERRIDES += \
    BuildDesc="sisleyr-user 5.0.2 LRX22G S90-A_S239_151126_ROW release-keys" \
    BuildFingerprint=Lenovo/sisleyr/sisleyr:5.0.2/LRX22G/S90-A_S239_151126_ROW:user/release-keys \
    DeviceName=sisleyr

# GMS
PRODUCT_GMS_CLIENTID_BASE := android-lenovo

# First api level, device has been commercially launched
PRODUCT_PROPERTY_OVERRIDES += \
    ro.product.first_api_level=19

# Device was launched with KK
$(call inherit-product, $(SRC_TARGET_DIR)/product/product_launched_with_k.mk)

# Use Etar as calendar
PRODUCT_PACKAGES += \
    Etar

# Makes device compatible with Google Dialer Go.
# In best case this file should be a part of Gapps
# but we don't ship Gapps in this target
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/googledialergo-sysconfig.xml:$(TARGET_COPY_OUT_SYSTEM)/etc/sysconfig/googledialergo-sysconfig.xml

# Lights
PRODUCT_PACKAGES += \
    android.hardware.light@2.0-service.a6000

# RIL
PRODUCT_PROPERTY_OVERRIDES += \
    persist.data.target=dpm1 \
    persist.radio.ecc_hard_1=112,911,110,122,119,120,000,118

$(call inherit-product, device/lenovo/a6010/sisleyr.mk)
