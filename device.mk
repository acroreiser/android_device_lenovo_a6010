#
# Copyright (C) 2016 The CyanogenMod Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Soong namespaces
PRODUCT_SOONG_NAMESPACES += \
    $(LOCAL_PATH)

# Boot animation
TARGET_SCREEN_HEIGHT := 1280
TARGET_SCREEN_WIDTH := 720

# AAPT CONFIG
PRODUCT_AAPT_CONFIG := normal
PRODUCT_AAPT_PREF_CONFIG := xhdpi

# Display
PRODUCT_PACKAGES += \
    disable_configstore \
    android.hardware.graphics.allocator@2.0-impl \
    android.hardware.graphics.allocator@2.0-service \
    android.hardware.graphics.mapper@2.0-impl-2.1 \
    android.hardware.graphics.composer@2.1-service \
    android.hardware.memtrack@1.0-impl \
    android.hardware.memtrack@1.0-service \
    gralloc.msm8916 \
    hwcomposer.msm8916 \
    libgenlock \
    libtinyxml \
    memtrack.msm8916

# RenderScript HAL
PRODUCT_PACKAGES += \
    android.hardware.renderscript@1.0-impl

# Properties
PRODUCT_PROPERTY_OVERRIDES += \
    ro.surface_flinger.max_frame_buffer_acquired_buffers=1 \
    ro.surface_flinger.force_hwc_copy_for_virtual_displays=true \
    ro.surface_flinger.has_HDR_display=false \
    ro.surface_flinger.has_wide_color_display=false \
    ro.surface_flinger.start_graphics_allocator_service=true \
    ro.surface_flinger.use_context_priority=true \
    debug.mdpcomp.idletime=600 \
    persist.hwc.mdpcomp.enable=true \
    persist.hwc.ptor.enable=true \
    debug.enable.sglscale=1 \
    debug.sf.hw=1 \
    debug.egl.hw=1 \
    debug.sf.disable_backpressure=1 \
    debug.sf.enable_gl_backpressure=1 \
    debug.sf.recomputecrop=0 \
    debug.cpurend.vsync=false \
    ro.config.avoid_gfx_accel=true \
    DEVICE_PROVISIONED=1 \
    ro.com.android.mobiledata=false \
    dev.pm.dyn_samplingrate=1 \
    persist.timed.enable=true \
    ro.hardware.egl=adreno \
    ro.surface_flinger.supports_background_blur=false \
    persist.sys.sf.disable_blurs=true \
    sys.use_fifo_ui=1

# Screen density
PRODUCT_PROPERTY_OVERRIDES += \
    ro.sf.lcd_density=280 \
    persist.graphics.vulkan.disable=true

# Disable buffer age
PRODUCT_PROPERTY_OVERRIDES += \
    debug.hwui.use_buffer_age=false

# DRM
PRODUCT_PACKAGES += \
    android.hardware.drm@1.0-impl \
    android.hardware.drm@1.0-service-lazy \
    android.hardware.drm@1.3-service-lazy.clearkey

# Netflix
PRODUCT_PROPERTY_OVERRIDES += \
    ro.netflix.bsp_rev=Q660-13149-1

PRODUCT_PROPERTY_OVERRIDES += \
    ro.opengles.version=196608

# Encryption
PRODUCT_PACKAGES += \
    vendor.qti.hardware.cryptfshw@1.0-base \
    vendor.qti.hardware.cryptfshw@1.0-service-qti.qsee \
    fde_ext4_resize.sh

# Audio
PRODUCT_PACKAGES += \
    audio.primary.msm8916 \
    audio.a2dp.default \
    audio.r_submix.default \
    audio.usb.default \
    tinyalsa \
    tinyplay \
    tinypcminfo \
    tinymix \
    libaudio-resampler \
    libqcomvisualizer \
    libqcomvoiceprocessing \
    libqcompostprocbundle \
    android.hardware.audio@6.0-impl \
    android.hardware.audio.service \
    android.hardware.audio.effect@6.0-impl

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/audio/audio_effects.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_effects.xml

# Audio
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/audio/acdb/QRD_Bluetooth_cal.acdb:system/etc/acdbdata/QRD/QRD_Bluetooth_cal.acdb \
    $(LOCAL_PATH)/audio/acdb/QRD_General_cal.acdb:system/etc/acdbdata/QRD/QRD_General_cal.acdb \
    $(LOCAL_PATH)/audio/acdb/QRD_Global_cal.acdb:system/etc/acdbdata/QRD/QRD_Global_cal.acdb \
    $(LOCAL_PATH)/audio/acdb/QRD_Handset_cal.acdb:system/etc/acdbdata/QRD/QRD_Handset_cal.acdb \
    $(LOCAL_PATH)/audio/acdb/QRD_Hdmi_cal.acdb:system/etc/acdbdata/QRD/QRD_Hdmi_cal.acdb \
    $(LOCAL_PATH)/audio/acdb/QRD_Headset_cal.acdb:system/etc/acdbdata/QRD/QRD_Headset_cal.acdb \
    $(LOCAL_PATH)/audio/acdb/QRD_Speaker_cal.acdb:system/etc/acdbdata/QRD/QRD_Speaker_cal.acdb \
    $(LOCAL_PATH)/audio/audio_platform_info.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_platform_info.xml \
    $(LOCAL_PATH)/audio/mixer_paths.xml:$(TARGET_COPY_OUT_VENDOR)/etc/mixer_paths_qrd_skuh.xml \
    $(LOCAL_PATH)/audio/audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/a2dp_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/a2dp_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/audio_policy_volumes.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_volumes.xml \
    frameworks/av/services/audiopolicy/config/default_volume_tables.xml:$(TARGET_COPY_OUT_VENDOR)/etc/default_volume_tables.xml \
    frameworks/av/services/audiopolicy/config/hearing_aid_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/hearing_aid_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/primary_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/primary_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/r_submix_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/r_submix_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/usb_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/usb_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/enginedefault/config/example/phone/audio_policy_engine_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_engine_configuration.xml \
    frameworks/av/services/audiopolicy/enginedefault/config/example/phone/audio_policy_engine_default_stream_volumes.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_engine_default_stream_volumes.xml \
    frameworks/av/services/audiopolicy/enginedefault/config/example/phone/audio_policy_engine_product_strategies.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_engine_product_strategies.xml \
    frameworks/av/services/audiopolicy/enginedefault/config/example/phone/audio_policy_engine_stream_volumes.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_engine_stream_volumes.xml

# Audio package for Go
$(call inherit-product, frameworks/base/data/sounds/AudioPackageGo.mk)

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.audio.low_latency.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.audio.low_latency.xml

# Ethernet
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.ethernet.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.ethernet.xml

# APEX
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/ld.config.txt:system/etc/swcodec/ld.config.txt

# Public Libraries
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/public.libraries.txt:$(TARGET_COPY_OUT_VENDOR)/etc/public.libraries.txt

# Enable iorapd perfetto tracing for app starts
# Enable iorapd readahead for app starts
PRODUCT_PROPERTY_OVERRIDES += \
    ro.iorapd.enable=false

# Properties
PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.audio.sdk.fluencetype=none \
    persist.vendor.audio.fluence.voicecall=true \
    persist.vendor.audio.fluence.voicerec=true \
    persist.vendor.audio.fluence.speaker=false \
    vendor.audio.offload_wakelock=false

# Properties
PRODUCT_PROPERTY_OVERRIDES += \
    audio.deep_buffer.media=true \
    audio.offload.min.duration.secs=30 \
    audio.offload.video=true \
    ro.af.client_heap_size_kbyte=7168 \
    ro.audio.flinger_standbytime_ms=300 \
    vendor.voice.path.for.pcm.voip=true \
    vendor.audio.av.streaming.offload.enable=true \
    vendor.audio.offload.buffer.size.kb=64 \
    ro.config.vc_call_vol_steps=7 \
    ro.config.media_vol_steps=25 \
    persist.bluetooth.bluetooth_audio_hal.disabled=true \
    ro.vendor.bluetooth.lazyhal=true \
    vendor.audio.offload.gapless.enabled=true

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.software.midi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.midi.xml

# Bluetooth
PRODUCT_PACKAGES += \
    libbt-vendor \

PRODUCT_COPY_FILES += \
    prebuilts/vndk/v28/arm/arch-arm-armv7-a-neon/shared/vndk-sp/libbase.so:$(TARGET_COPY_OUT_VENDOR)/lib/libbase-v28.so

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.bluetooth.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth.xml \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth_le.xml

# Camera
PRODUCT_PACKAGES += \
    android.hardware.camera.provider@2.5-impl \
    android.hardware.camera.provider@2.5-service \
    camera.device@1.0-impl \
    camera.msm8916 \
    libmm-qcamera \
    libshim_atomic \
    libboringssl-compat \
    Snap

PRODUCT_PROPERTY_OVERRIDES += \
    persist.camera.cpp.duplication=false \
    persist.camera.hal.debug.mask=0 \
    persist.camera.feature.cac=1

# FastCharge
PRODUCT_PACKAGES += \
    vendor.lineage.fastcharge@1.0-service.usb

# Screen Recorder
PRODUCT_PACKAGES += \
    Recorder

# Tethering all-in-one
PRODUCT_PROPERTY_OVERRIDES += \
    sys.fflag.override.settings_tether_all_in_one=true

# Seamless transfer
PRODUCT_PROPERTY_OVERRIDES += \
    sys.fflag.override.settings_seamless_transfer=true

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.front.xml

# First stage init
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/rootdir/etc/fstab.ramdisk.qcom:$(TARGET_COPY_OUT_RAMDISK)/fstab.qcom

# RIL
PRODUCT_PACKAGES += \
   libaudioclient_shim \
   libcutils_shim

# Charger
PRODUCT_SYSTEM_DEFAULT_PROPERTIES += \
    ro.charger.enable_suspend=true

# Charger images
PRODUCT_PACKAGES += charger_res_images

# GPS
PRODUCT_PACKAGES += \
    libandroid_net \
    libcurl

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/gps/apdr.conf:system/vendor/etc/apdr.conf \
    $(LOCAL_PATH)/gps/flp.conf:system/vendor/etc/flp.conf \
    $(LOCAL_PATH)/gps/gps.conf:system/vendor/etc/gps.conf \
    $(LOCAL_PATH)/gps/izat.conf:system/vendor/etc/izat.conf \
    $(LOCAL_PATH)/gps/sap.conf:system/vendor/etc/sap.conf \
    $(LOCAL_PATH)/gps/lowi.conf:system/vendor/etc/lowi.conf \
    $(LOCAL_PATH)/gps/xtwifi.conf:system/vendor/etc/xtwifi.conf

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.location.gps.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.location.gps.xml

# Overlay
DEVICE_PACKAGE_OVERLAYS += \
    $(LOCAL_PATH)/overlay \
    $(LOCAL_PATH)/overlay-lineage

# Enforced RRO targets
PRODUCT_ENFORCE_RRO_TARGETS := *
PRODUCT_ENFORCE_RRO_EXCLUDED_OVERLAYS += \
    $(LOCAL_PATH)/overlay/packages/apps/Snap \
    $(LOCAL_PATH)/overlay-lineage/lineage-sdk

# Overlay for Gboard
PRODUCT_PACKAGES += \
    GboardOverlay

# Overlay for Google Messages
PRODUCT_PACKAGES += \
    MessagesGoOverlay

# DocumentsUI overlay
PRODUCT_PACKAGES += \
    DocumentsUIOverlay

# Trebuchet overlay
PRODUCT_PACKAGES += \
    TrebuchetGoOverlay

# Properties
PRODUCT_PROPERTY_OVERRIDES += \
    persist.bluetooth.disableabsvol=true \
    vendor.qcom.bluetooth.soc=pronto \
    ro.bluetooth.dun=true \
    ro.bluetooth.hfp.ver=1.7 \
    ro.bluetooth.sap=true \
    vendor.bluetooth.soc=pronto \
    ro.qualcomm.bt.hci_transport=pronto

# Keymaster HAL
PRODUCT_PACKAGES += \
    android.hardware.keymaster@4.1-service \
    android.hardware.gatekeeper@1.0-service.software

# Doze
PRODUCT_PACKAGES += \
    LenovoDoze

# FM
PRODUCT_PACKAGES += \
    FMRadio \
    libfmjni

# Google Assistant
PRODUCT_PROPERTY_OVERRIDES += \
    ro.opa.eligible_device=true

# Init scripts
PRODUCT_PACKAGES += \
    fstab.qcom \
    init.target.rc \
    init.qcom.rc \
    init.qcom.power.rc \
    init.qcom.ssr.rc \
    init.qcom.usb.rc \
    init.recovery.qcom.rc \
    ueventd.qcom.rc

# For config.fs
PRODUCT_PACKAGES += \
    fs_config_files

# Media
PRODUCT_COPY_FILES += \
    frameworks/av/media/libstagefright/data/media_codecs_google_audio.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_telephony.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_telephony.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_video.xml

# Media
PRODUCT_PACKAGES += \
    libextmedia_jni \
    libmm-omxcore \
    libOmxAacEnc \
    libOmxAmrEnc \
    libOmxCore \
    libOmxEvrcEnc \
    libOmxG711Enc \
    libOmxQcelp13Enc \
    libOmxVdec \
    libOmxVdecHevc \
    libOmxSwVencHevc \
    libOmxVenc \
    libOmxVidcCommon \
    libstagefrighthw \
    android.hardware.cas@1.2-service-lazy

# Media config
PRODUCT_COPY_FILES += \
    frameworks/av/media/libstagefright/data/media_codecs_google_video_le.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_video_le.xml \
    $(LOCAL_PATH)/configs/media_codecs.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs.xml

# Media
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/media_profiles_V1_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_profiles_V1_0.xml \
    $(LOCAL_PATH)/configs/media_codecs_performance.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_performance.xml

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.telephony.gsm.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/native/data/etc/android.hardware.telephony.cdma.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.cdma.xml \
    frameworks/native/data/etc/android.software.sip.voip.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.sip.voip.xml

PRODUCT_PACKAGES += \
    android.hardware.power-service-qti \
    android.hardware.power.stats@1.0-service.mock

# Perf
PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.extension_library=libqti-perfd-client.so

# IOP
PRODUCT_PROPERTY_OVERRIDES += \
    vendor.enable_prefetch=1 \
    vendor.iop.enable_uxe=1 \
    vendor.iop.enable_prefetch_ofr=1 \
    vendor.perf.iop_v3.enable=1 \
    persist.vendor.qti.games.gt.prof=1

# IRSC
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/sec_config:$(TARGET_COPY_OUT_VENDOR)/etc/sec_config

# First api level, device has been commercially launched
PRODUCT_PROPERTY_OVERRIDES += \
    ro.product.first_api_level=21

# Properties
PRODUCT_PROPERTY_OVERRIDES += \
    persist.data.qmi.adb_logmask=0 \
    persist.radio.apm_sim_not_pwdn=1 \
    persist.radio.add_power_save=1 \
    ro.telephony.call_ring.multiple=false

# HIDL
PRODUCT_PACKAGES += \
    android.hidl.base@1.0 \
    android.hidl.base@1.0_system \
    android.hidl.manager@1.0 \
    android.hidl.manager@1.0_system

# Binder
PRODUCT_PACKAGES += \
    libhwbinder \
    libhwbinder.vendor \
    libhidltransport \
    libhidltransport.vendor \
    libjson \
    libjson.vendor \
    vndservicemanager

# Health
PRODUCT_PACKAGES += \
    android.hardware.health@2.1-impl \
    android.hardware.health@2.1-service

# Touchscreen
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.usb.host.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.accessory.xml

# Vibrator
PRODUCT_PACKAGES += \
    android.hardware.vibrator@1.1-service.msm8916

# RIL
PRODUCT_PACKAGES += \
    libcnefeatureconfig \
    librmnetctl \
    libxml2

# 1) Baseband version fix, 2) zram setup,
# 3) boosting after boot up, 4) set device dependement props 
PRODUCT_PACKAGES += \
    set_baseband.sh \
    set_zram.sh \
    init.boot_boost.sh \
    target_config.sh

# Lights
PRODUCT_PACKAGES += \
    android.hardware.light@2.0-service.msm8916

# Properties
PRODUCT_PROPERTY_OVERRIDES += \
    drm.service.enabled=1 \
    debug.stagefright.omx_default_rank.sw-audio=1 \
    debug.stagefright.omx_default_rank=0 \
    vendor.mediacodec.binder.size=4 \
    media.stagefright.thumbnail.prefer_hw_codecs=true \
    debug.stagefright.ccodec=0 \
    vidc.enc.narrow.searchrange=1

# Keylayout
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/keylayout/gpio-keys.kl:system/usr/keylayout/gpio-keys.kl \
    $(LOCAL_PATH)/keylayout/ft5x06_ts.kl:system/usr/keylayout/ft5x06_ts.kl\
    $(LOCAL_PATH)/keylayout/qpnp_pon.kl:system/usr/keylayout/qpnp_pon.kl

# Disable oem unlock
PRODUCT_PROPERTY_OVERRIDES += \
    ro.oem_unlock_supported=false

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.wifi.passpoint.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.passpoint.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.software.ipsec_tunnels.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.ipsec_tunnels.xml

# Properties
PRODUCT_PROPERTY_OVERRIDES += \
    wifi.interface=wlan0 \
    wifi.direct.interface=p2p0

# WiFi HAL
PRODUCT_PACKAGES += \
    android.hardware.wifi@1.0-service-lazy

# Wifi
PRODUCT_PACKAGES += \
    libwcnss_qmi \
    wcnss_service \
    hostapd \
    wpa_supplicant \
    wpa_supplicant.conf \
    libwpa_client \
    wpa_cli \
    WifiOverlay \
    TetheringConfigOverlay

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/p2p_supplicant_overlay.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/p2p_supplicant_overlay.conf \
    $(LOCAL_PATH)/configs/wpa_supplicant_overlay.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/wpa_supplicant_overlay.conf \
    $(LOCAL_PATH)/configs/WCNSS_qcom_cfg.ini:$(TARGET_COPY_OUT_VENDOR)/firmware/wlan/prima/WCNSS_qcom_cfg.ini \
    $(LOCAL_PATH)/wifi/WCNSS_cfg.dat:$(TARGET_COPY_OUT_VENDOR)/firmware/wlan/prima/WCNSS_cfg.dat \
    $(LOCAL_PATH)/wifi/WCNSS_qcom_wlan_nv.bin:$(TARGET_COPY_OUT_VENDOR)/firmware/wlan/prima/WCNSS_qcom_wlan_nv.bin \
    $(LOCAL_PATH)/wifi/WCNSS_wlan_dictionary.dat:$(TARGET_COPY_OUT_VENDOR)/firmware/wlan/prima/WCNSS_wlan_dictionary.dat

# WFD
PRODUCT_PROPERTY_OVERRIDES += \
    persist.debug.wfd.enable=1 \
    persist.sys.wfd.virtual=0

# TimeKeep
PRODUCT_PACKAGES += \
    timekeep \
    TimeKeep

# USB HAL
PRODUCT_PACKAGES += \
    android.hardware.usb@1.0-service.cyanogen_8916

# Optimize
PRODUCT_DEX_PREOPT_BOOT_IMAGE_PROFILE_LOCATION := $(LOCAL_PATH)/profiles/boot-image.prof
PRODUCT_DEX_PREOPT_GENERATE_DM_FILES := true
DEX2OAT_TARGET_INSTRUCTION_SET_FEATURES := div

# To leverage the P feature "Full Stack Integrity: expand verified boot to cover privapps" Android will
#  uncompress priv-apps for system APKs. Unfortunately this causes ~90MB increase system ROM size.
# For Go devices, you may disable this feature to save space.
DONT_UNCOMPRESS_PRIV_APPS_DEXS := true

# Animation
TARGET_BOOT_ANIMATION_RES := 720
TARGET_BOOTANIMATION_HALF_RES := true
TARGET_BOOTANIMATION_MULTITHREAD_DECODE := true
TARGET_BOOTANIMATION_PRELOAD := true
TARGET_BOOTANIMATION_TEXTURE_CACHE := true

# Strip debug
PRODUCT_MINIMIZE_JAVA_DEBUG_INFO := true

PRODUCT_PROPERTY_OVERRIDES += \
    dalvik.vm.zygotemaxfailedboots=5 \
    dalvik.vm.foreground-heap-growth-multiplier=2.0 \
    dalvik.vm.dex2oat-flags=--no-watch-dog \
    dalvik.vm.dex2oat-swap=false \
    dalvik.vm.boot-dex2oat-threads=4 \
    dalvik.vm.boot-dex2oat-cpu-set=0,1,2,3 \
    dalvik.vm.dex2oat-threads=2 \
    dalvik.vm.dex2oat-cpu-set=0,1 \
    ro.vendor.qti.am.reschedule_service=true \
    dalvik.vm.isa.arm.features=div \
    persist.sys.dalvik.vm.lib.2=libart.so \
    dalvik.vm.systemservercompilerfilter=speed-profile \
    dalvik.vm.minidebuginfo=false \
    dalvik.vm.dex2oat-minidebuginfo=false

# Improve scrolling
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.scrollingcache=0 \
    ro.min.fling_velocity=160 \
    ro.max.fling_velocity=20000

# IO Cgroups
PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.iocgrp.config=1

# Low Memory Killer
PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.qti.config.ulmk_memcg=true \
    ro.lmk.use_psi=false \
    ro.lmk.critical=0 \
    ro.lmk.low=950 \
    ro.lmk.swap_free_low_percentage=15

# Inherit customized Android Go defaults.
$(call inherit-product, device/lenovo/a6010/go_defaults_custom.mk)

# Setup dalvik vm configs
$(call inherit-product, frameworks/native/build/phone-hdpi-512-dalvik-heap.mk)

# Memory optimizations
PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.qti.sys.fw.bservice_enable=true \
    ro.vendor.qti.sys.fw.bservice_limit=5 \
    ro.vendor.qti.sys.fw.bservice_age=5000 \
    ro.vendor.qti.sys.fw.use_trim_settings=true \
    ro.vendor.qti.sys.fw.empty_app_percent=50 \
    ro.vendor.qti.sys.fw.trim_empty_percent=100 \
    ro.vendor.qti.sys.fw.trim_cache_percent=100 \
    ro.config.max_starting_bg=1

# Dexopt
PRODUCT_PROPERTY_OVERRIDES += \
    pm.dexopt.first-boot=quicken \
    pm.dexopt.boot=extract

# Boot
PRODUCT_PROPERTY_OVERRIDES += \
    sys.vendor.shutdown.waittime=500 \
    ro.build.shutdown_timeout=0

# Battery
PRODUCT_PROPERTY_OVERRIDES += \
    ro.config.small_battery=true

# TextClassifier
PRODUCT_COPY_FILES += \
    external/libtextclassifier/native/models/textclassifier.ru.model:$(TARGET_COPY_OUT_SYSTEM)/etc/textclassifier/textclassifier.ru.model

# Properties
PRODUCT_PROPERTY_OVERRIDES += \
    keyguard.no_require_sim=true \
    persist.data.qmi.adb_logmask=0 \
    persist.radio.apm_sim_not_pwdn=1 \
    ro.telephony.call_ring.multiple=false \
    ro.use_data_netmgrd=true \
    persist.radio.multisim.config=dsds \
    persist.radio.custom_ecc=1 \
    ril.ecclist=000,08,100,101,102,110,112,118,119,120,122,911,999 \
    ril.ecclist1=000,08,100,101,102,110,112,118,119,120,122,911,999 \
    persist.radio.ecc_hard_count=1 \
    rild.libpath=/system/vendor/lib/libril-qc-qmi-1.so \
    ril.subscription.types=NV,RUIM \
    ro.telephony.default_network=9,1 \
    persist.vendor.radio.add_power_save=1 \
    persist.sys.fflag.override.settings_network_and_internet_v2=true \
    persist.data.netmgrd.qos.enable=true

# RIL
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/data/netmgr_config.xml:$(TARGET_COPY_OUT_VENDOR)/etc/data/netmgr_config.xml \
    $(LOCAL_PATH)/configs/data/qmi_config.xml:$(TARGET_COPY_OUT_VENDOR)/etc/data/qmi_config.xml \
    $(LOCAL_PATH)/configs/data/dsi_config.xml:$(TARGET_COPY_OUT_VENDOR)/etc/data/dsi_config.xml

# Use Etar as calendar
PRODUCT_PACKAGES += \
    Etar

# Seccomp
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/seccomp/mediacodec-seccomp.policy:$(TARGET_COPY_OUT_VENDOR)/etc/seccomp_policy/mediacodec.policy \
    $(LOCAL_PATH)/seccomp/mediaextractor-seccomp.policy:$(TARGET_COPY_OUT_VENDOR)/etc/seccomp_policy/mediaextractor.policy \
    $(LOCAL_PATH)/seccomp/mediaswcodec-seccomp.policy:system/vendor/etc/seccomp_policy/mediaswcodec.policy

# Thermal HAL
PRODUCT_PACKAGES += \
    android.hardware.thermal@2.0-service.msm8916

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/thermal_info_config.json:$(TARGET_COPY_OUT_VENDOR)/etc/thermal_info_config.json \
    $(LOCAL_PATH)/configs/thermal-engine.conf:$(TARGET_COPY_OUT_VENDOR)/etc/thermal-engine.conf

PRODUCT_PROPERTY_OVERRIDES += \
    vendor.thermal.config=thermal_info_config.json

# Sensors
PRODUCT_PACKAGES += \
    calmodule.cfg \
    libcalmodule_common \
    libcalmodule_akm \
    android.hardware.sensors@1.0-impl.msm8916 \
    android.hardware.sensors@1.0-service.msm8916 \
    accelcal \
    AccCalibration \
    sensord \
    sensors.msm8916

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.proximity.xml \
    $(LOCAL_PATH)/configs/sensors/_hals.conf:$(TARGET_COPY_OUT_VENDOR)/etc/sensors/_hals.conf

# LiveDisplay
PRODUCT_PACKAGES += \
    vendor.lineage.livedisplay@2.0-service-sysfs

# Touch
PRODUCT_PACKAGES += \
    vendor.lineage.touch@1.0-service.a6010

# AOSP filesystems
PRODUCT_PACKAGES += \
    fsck.exfat \
    fsck.ntfs \
    mke2fs \
    mkfs.exfat \
    mkfs.ntfs \
    mount.ntfs

# Updater properties
PRODUCT_PROPERTY_OVERRIDES += \
    persist.vendor.recovery_update=false

# USB ID
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    ro.usb.id.midi=90BA \
    ro.usb.id.midi_adb=90BB \
    ro.usb.id.mtp=2281 \
    ro.usb.id.mtp_adb=2282 \
    ro.usb.id.ptp=2284 \
    ro.usb.id.ptp_adb=2283 \
    ro.usb.id.ums=2286 \
    ro.usb.id.ums_adb=2285 \
    ro.usb.vid=2970

# Device was launched with L
$(call inherit-product, $(SRC_TARGET_DIR)/product/product_launched_with_l.mk)

# Privapp-permissions whitelisting
PRODUCT_PROPERTY_OVERRIDES += \
    ro.control_privapp_permissions=enforce

# Since ($SRC_TARGET)/product/generic.mk is included instead of full_base.mk the device config also need to  
#  pick up the default Android Platform product locale list
$(call inherit-product, $(SRC_TARGET_DIR)/product/languages_full.mk)

$(call inherit-product, $(SRC_TARGET_DIR)/product/emulated_storage.mk)

$(call inherit-product, vendor/lenovo/a6010/a6010-vendor.mk)
