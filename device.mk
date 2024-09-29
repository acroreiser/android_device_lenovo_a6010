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
    libtinyxml \
    memtrack.msm8916

# RenderScript HAL
PRODUCT_PACKAGES += \
    android.hardware.renderscript@1.0-impl

# Properties
PRODUCT_PROPERTY_OVERRIDES += \
    ro.surface_flinger.max_frame_buffer_acquired_buffers=3 \
    ro.surface_flinger.force_hwc_copy_for_virtual_displays=true \
    ro.surface_flinger.use_color_management=false \
    ro.surface_flinger.has_HDR_display=false \
    ro.surface_flinger.has_wide_color_display=false \
    ro.surface_flinger.enable_layer_caching=false \
    ro.surface_flinger.start_graphics_allocator_service=true \
    ro.surface_flinger.use_context_priority=true \
    ro.surface_flinger.clear_slots_with_set_layer_buffer=true \
    debug.sf.enable_planner_prediction=false \
    debug.sf.disable_client_composition_cache=1 \
    debug.sf.early_phase_offset_ns=1500000 \
    debug.sf.early_app_phase_offset_ns=1500000 \
    debug.sf.early_gl_phase_offset_ns=3000000 \
    debug.sf.early_gl_app_phase_offset_ns=15000000 \
    ro.surface_flinger.set_idle_timer_ms=80 \
    ro.surface_flinger.set_touch_timer_ms=200 \
    ro.surface_flinger.set_display_power_timer_ms=1000 \
    ro.surface_flinger.vsync_event_phase_offset_ns=1000000 \
    ro.surface_flinger.vsync_sf_event_phase_offset_ns=1000000 \
    ro.surface_flinger.support_kernel_idle_timer=true \
    debug.mdpcomp.idletime=600 \
    persist.hwc.mdpcomp.enable=true \
    persist.hwc.ptor.enable=true \
    debug.enable.sglscale=1 \
    debug.sf.hw=0 \
    debug.egl.hw=0 \
    debug.sf.disable_backpressure=1 \
    debug.sf.enable_gl_backpressure=1 \
    debug.sf.auto_latch_unsignaled=0 \
    debug.sf.latch_unsignaled=0 \
    debug.sf.recomputecrop=0 \
    debug.cpurend.vsync=false \
    debug.sf.layer_caching_active_layer_timeout_ms=1000 \
    ro.config.avoid_gfx_accel=true \
    DEVICE_PROVISIONED=1 \
    ro.com.android.mobiledata=false \
    dev.pm.dyn_samplingrate=1 \
    persist.timed.enable=true \
    ro.hardware.egl=adreno \
    ro.surface_flinger.supports_background_blur=false \
    persist.sys.sf.color_saturation=1.0 \
    persist.sys.sf.disable_blurs=true \
    debug.renderengine.backend=gles \
    debug.hwui.renderer=opengl \
    ro.hwui.render_ahead=20 \
    ro.config.night_mode_on_battery_saver=false

# Disable surfaceflinger prime_shader cache to improve post boot memory.
PRODUCT_PROPERTY_OVERRIDES += service.sf.prime_shader_cache=0

# Screen density
PRODUCT_PROPERTY_OVERRIDES += \
    ro.sf.lcd_density=280 \
    persist.graphics.vulkan.disable=true

# Disable buffer age
PRODUCT_PROPERTY_OVERRIDES += \
    debug.hwui.use_buffer_age=false

# Enable support of one-handed mode
PRODUCT_PROPERTY_OVERRIDES += \
    ro.support_one_handed_mode=true

# UI and LMK configs from
# Nokia C02 Nokia/Cypher_00WW/CYR:12/SP1A.210812.016/00WW_1_350:user/release-keys
PRODUCT_PROPERTY_OVERRIDES += \
    sys.use_fifo_ui=1 \
    ro.config.per_app_memcg=false \
    ro.lmk.psi_complete_stall_ms=400 \
    ro.lmk.filecache_min_kb=110592

# Avoid thrashing to prevent freezes.
# LMKD will not be tolerant to thrashing at all.
# This will impact multitasking and cached apps.
PRODUCT_PROPERTY_OVERRIDES += \
    ro.lmk.thrashing_limit=0

# Bluetooth
PRODUCT_PACKAGES += \
    android.hardware.bluetooth@1.0

# DRM
PRODUCT_PACKAGES += \
    android.hardware.drm@1.0-impl \
    android.hardware.drm@1.0-service-lazy \
    android.hardware.drm@1.4-service-lazy.clearkey

PRODUCT_PROPERTY_OVERRIDES += \
    ro.opengles.version=196608

# Audio
PRODUCT_PACKAGES += \
    audio.primary.msm8916 \
    audio.r_submix.default \
    audio.usb.default \
    audio.usbv2.default \
    tinyplay \
    tinypcminfo \
    tinymix \
    libaudio-resampler \
    libqcomvisualizer \
    libqcomvoiceprocessing \
    libqcompostprocbundle \
    android.hardware.audio@7.1-impl \
    android.hardware.audio.service \
    android.hardware.audio.effect@7.0-impl

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/audio/audio_effects.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_effects.xml

# Audio
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/audio/acdb/QRD_Bluetooth_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/QRD/QRD_Bluetooth_cal.acdb \
    $(LOCAL_PATH)/audio/acdb/QRD_Handset_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/QRD/QRD_Handset_cal.acdb \
    $(LOCAL_PATH)/audio/acdb/QRD_Hdmi_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/QRD/QRD_Hdmi_cal.acdb \
    $(LOCAL_PATH)/audio/acdb/QRD_Headset_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/QRD/QRD_Headset_cal.acdb \
    $(LOCAL_PATH)/audio/audio_platform_info.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_platform_info.xml \
    $(LOCAL_PATH)/audio/mixer_paths.xml:$(TARGET_COPY_OUT_VENDOR)/etc/mixer_paths_qrd_skuh.xml \
    $(LOCAL_PATH)/audio/mixer_paths.xml:$(TARGET_COPY_OUT_VENDOR)/etc/mixer_paths_qrd_skui.xml \
    $(LOCAL_PATH)/audio/audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_configuration.xml \
    $(LOCAL_PATH)/audio/audio_output_policy.conf:$(TARGET_COPY_OUT_VENDOR)/etc/audio_output_policy.conf \
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

# SoC
PRODUCT_PROPERTY_OVERRIDES += \
	ro.soc.manufacturer=Qualcomm \
	ro.soc.model=MSM8916

# Public Libraries
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/public.libraries.txt:$(TARGET_COPY_OUT_VENDOR)/etc/public.libraries.txt

# Cgroup and task_profiles for freezer on cgroups v1
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/cgroups.json:$(TARGET_COPY_OUT_VENDOR)/etc/cgroups.json \
    $(LOCAL_PATH)/configs/task_profiles.json:$(TARGET_COPY_OUT_VENDOR)/etc/task_profiles.json

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
    persist.bluetooth.bluetooth_audio_hal.disabled=false \
    ro.bluetooth.a2dp_offload.supported=false \
    persist.bluetooth.a2dp_offload.disabled=true \
    ro.vendor.bluetooth.lazyhal=true \
    vendor.audio.offload.gapless.enabled=true \
    bluetooth.device.class_of_device=90,2,12 \
    bluetooth.profile.a2dp.source.enabled=true \
    bluetooth.profile.asha.central.enabled=false \
    bluetooth.profile.avrcp.target.enabled=true \
    bluetooth.profile.bas.client.enabled=true \
    bluetooth.profile.gatt.enabled=true \
    bluetooth.profile.hfp.ag.enabled=true \
    bluetooth.profile.hid.device.enabled=true \
    bluetooth.profile.hid.host.enabled=true \
    bluetooth.profile.map.server.enabled=true \
    bluetooth.profile.opp.enabled=true \
    bluetooth.profile.pan.nap.enabled=true \
    bluetooth.profile.pan.panu.enabled=true \
    bluetooth.profile.pbap.server.enabled=true \
    bluetooth.profile.sap.server.enabled=true \
    bluetooth.le.disable_apcf_extended_features=1

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.audio.low_latency.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.audio.low_latency.xml \
    frameworks/native/data/etc/android.hardware.bluetooth.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth.xml \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth_le.xml \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.ethernet.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.ethernet.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/native/data/etc/android.hardware.telephony.gsm.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/native/data/etc/android.hardware.telephony.cdma.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.cdma.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/android.hardware.wifi.passpoint.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.passpoint.xml \
    frameworks/native/data/etc/android.software.ipsec_tunnels.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.ipsec_tunnels.xml \
    frameworks/native/data/etc/android.software.midi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.midi.xml \
    frameworks/native/data/etc/android.software.sip.voip.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.sip.voip.xml

# Camera
PRODUCT_PACKAGES += \
    android.hardware.camera.provider@2.5-service \
    camera.device@1.0-impl \
    camera.msm8916 \
    camera.legacy.msm8916 \
    libmm-qcamera \
    Aperture

# Camera experimental libs
PRODUCT_PACKAGES += \
    libmmcamera_ov13850_experimental \
    libchromatix_ov13850_snapshot_experimental

PRODUCT_PROPERTY_OVERRIDES += ro.camera.experimental_libs=true

PRODUCT_PROPERTY_OVERRIDES += \
    persist.camera.cpp.duplication=false \
    persist.camera.hal.debug.mask=0 \
    persist.camera.feature.cac=1 \
    persist.camera.no-display=1 \
    persist.camera.hal3on1.use_limited_level=1

# FastCharge
PRODUCT_PACKAGES += \
    vendor.lineage.fastcharge@1.0-service.usb

# Screen Recorder
PRODUCT_PACKAGES += \
    Recorder

# Repainter integration
PRODUCT_PACKAGES += \
    RepainterServicePriv

# Seamless transfer
PRODUCT_PROPERTY_OVERRIDES += \
    sys.fflag.override.settings_seamless_transfer=true

# System
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.binary_xml=false

# First stage init
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/rootdir/etc/fstab.ramdisk.qcom:$(TARGET_COPY_OUT_RAMDISK)/fstab.qcom

# Charger
PRODUCT_SYSTEM_DEFAULT_PROPERTIES += \
    ro.charger.enable_suspend=true

# Charger images
PRODUCT_PACKAGES += charger_res_images

# GPS
PRODUCT_PACKAGES += \
    libandroid_net \
    libcurl \
    libsensorndkbridge \
    xtra_daemon_toggler.sh

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/gps/apdr.conf:$(TARGET_COPY_OUT_VENDOR)/etc/apdr.conf \
    $(LOCAL_PATH)/configs/gps/flp.conf:$(TARGET_COPY_OUT_VENDOR)/etc/flp.conf \
    $(LOCAL_PATH)/configs/gps/gps.conf:$(TARGET_COPY_OUT_VENDOR)/etc/gps.conf \
    $(LOCAL_PATH)/configs/gps/izat.conf:$(TARGET_COPY_OUT_VENDOR)/etc/izat.conf \
    $(LOCAL_PATH)/configs/gps/izat_xtra.conf:$(TARGET_COPY_OUT_VENDOR)/etc/izat_xtra.conf \
    $(LOCAL_PATH)/configs/gps/lowi.conf:$(TARGET_COPY_OUT_VENDOR)/etc/lowi.conf \
    $(LOCAL_PATH)/configs/gps/sap.conf:$(TARGET_COPY_OUT_VENDOR)/etc/sap.conf \
    $(LOCAL_PATH)/configs/gps/xtwifi.conf:$(TARGET_COPY_OUT_VENDOR)/etc/xtwifi.conf

# Overlay
DEVICE_PACKAGE_OVERLAYS += \
    $(LOCAL_PATH)/overlay \
    $(LOCAL_PATH)/overlay-lineage

# Enforced RRO targets
PRODUCT_ENFORCE_RRO_TARGETS := *
PRODUCT_ENFORCE_RRO_EXCLUDED_OVERLAYS += \
    $(LOCAL_PATH)/overlay-lineage/lineage-sdk

# Overlay for Gboard
PRODUCT_PACKAGES += \
    GboardOverlay

# Overlay for Google Messages
PRODUCT_PACKAGES += \
    MessagesGoOverlay

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
    ro.qualcomm.bt.hci_transport=pronto \
    bluetooth.core.le.vendor_capabilities.enabled=false

# Incremental FS
PRODUCT_PROPERTY_OVERRIDES += \
    ro.incremental.enable=1

# Keymaster HAL
PRODUCT_PACKAGES += \
    android.hardware.keymaster@4.1-service \
    android.hardware.gatekeeper@1.0-service.software

# FM
PRODUCT_PACKAGES += \
    RevampedFMRadio \
    libqcomfmjni

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
    libmm-omxcore \
    libOmxAacEnc \
    libOmxAmrEnc \
    libOmxCore \
    libOmxEvrcEnc \
    libOmxQcelp13Enc \
    libOmxVdec \
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

# Disable media transcoding
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.fuse.transcode_user_control=true \
    persist.sys.fuse.transcode_enabled=false

PRODUCT_PACKAGES += \
    android.hardware.power-service-qti \
    android.hardware.power.stats@1.0-service.mock

# Music player
PRODUCT_PACKAGES += \
    Eleven \
    AudioFX


# Perf
PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.extension_library=libqti-perfd-client.so

# IRSC
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/sec_config:$(TARGET_COPY_OUT_VENDOR)/etc/sec_config

# Properties
PRODUCT_PROPERTY_OVERRIDES += \
    persist.data.qmi.adb_logmask=0 \
    persist.radio.apm_sim_not_pwdn=1 \
    persist.radio.add_power_save=1 \
    ro.telephony.call_ring.multiple=false \
    ro.telephony.iwlan_operation_mode=legacy

# HIDL
PRODUCT_PACKAGES += \
    android.hidl.base@1.0 \
    android.hidl.manager@1.0 \

PRODUCT_SYSTEM_DEFAULT_PROPERTIES += \
    debug.fdsan=0

# Binder
PRODUCT_PACKAGES += \
    libhwbinder \
    libhidltransport \
    libjson \
    vndservicemanager

# Health
PRODUCT_PACKAGES += \
    android.hardware.health@2.1-impl \
    android.hardware.health@2.1-service

# Lineage Health
PRODUCT_PACKAGES += \
    vendor.lineage.health-service.default

# Vibrator
PRODUCT_PACKAGES += \
    android.hardware.vibrator@1.1-service.msm8916

# RIL
PRODUCT_PACKAGES += \
    librmnetctl \
    libxml2

# Net
PRODUCT_PACKAGES += \
    netutils-wrapper-1.0

PRODUCT_PACKAGES += \
    android.hardware.radio.config@1.0-service

# Configuring device during boot
PRODUCT_PACKAGES += \
    set_baseband.sh \
    set_tasks.sh \
    set_zram.sh \
    init.post_boot.sh \
    target_config.sh

# msm_irqbalance
PRODUCT_PACKAGES += \
    msm_irqbalance

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/msm_irqbalance.conf:$(TARGET_COPY_OUT_VENDOR)/etc/msm_irqbalance.conf

# Properties
PRODUCT_PROPERTY_OVERRIDES += \
    drm.service.enabled=1 \
    debug.stagefright.omx_default_rank.sw-audio=1 \
    debug.stagefright.omx_default_rank=0 \
    vendor.mediacodec.binder.size=4 \
    media.stagefright.thumbnail.prefer_hw_codecs=true \
    debug.stagefright.ccodec=0 \
    vidc.enc.narrow.searchrange=1 \
    ro.media.maxmem=268435456

# Keylayout
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/keylayout/gpio-keys.kl:system/usr/keylayout/gpio-keys.kl \
    $(LOCAL_PATH)/keylayout/ft5x06_ts.kl:system/usr/keylayout/ft5x06_ts.kl\
    $(LOCAL_PATH)/keylayout/qpnp_pon.kl:system/usr/keylayout/qpnp_pon.kl

# Disable oem unlock
PRODUCT_PROPERTY_OVERRIDES += \
    ro.oem_unlock_supported=0

# Properties
PRODUCT_PROPERTY_OVERRIDES += \
    wifi.interface=wlan0 \
    wifi.direct.interface=p2p0

# WiFi HAL
PRODUCT_PACKAGES += \
    android.hardware.wifi@1.0-service-lazy

# Wifi
PRODUCT_PACKAGES += \
    wcnss_service \
    hostapd \
    wpa_supplicant \
    wpa_supplicant.conf \
    libwpa_client \
    wpa_cli \
    WifiOverlay \
    TetheringConfigOverlay \
    iw

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
PRODUCT_DEX_PREOPT_GENERATE_DM_FILES := true
DEX2OAT_TARGET_INSTRUCTION_SET_FEATURES := div

# To leverage the P feature "Full Stack Integrity: expand verified boot to cover privapps" Android will
#  uncompress priv-apps for system APKs. Unfortunately this causes ~90MB increase system ROM size.
# For Go devices, you may disable this feature to save space.
DONT_UNCOMPRESS_PRIV_APPS_DEXS := true

# SystemUI module for Android Go
PRODUCT_PACKAGES += \
    SystemUIGo

# Animation
TARGET_BOOT_ANIMATION_RES := 720
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
    persist.sys.dalvik.vm.lib.2=libart.so \
    dalvik.vm.systemservercompilerfilter=speed-profile

# Improve scrolling
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.scrollingcache=0 \
    ro.min.fling_velocity=160 \
    ro.max.fling_velocity=20000

# IO Cgroups
PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.iocgrp.config=1

# Zygote
PRODUCT_PROPERTY_OVERRIDES += \
    zygote.critical_window.minute=10

# Inherit customized Android Go defaults.
$(call inherit-product, device/lenovo/a6010/go_defaults_custom.mk)

# Setup dalvik vm configs
$(call inherit-product, frameworks/native/build/phone-hdpi-512-dalvik-heap.mk)

# Memory optimizations
PRODUCT_PROPERTY_OVERRIDES += \
    ro.config.max_starting_bg=1 \
    ro.am.no_kill_cached_processes_until_boot_completed=false \
    ro.am.no_kill_cached_processes_post_boot_completed_duration_millis=0 \
    ro.hw_timeout_multiplier=6

# Dexopt
PRODUCT_PROPERTY_OVERRIDES += \
    pm.dexopt.install=speed-profile \
    pm.dexopt.bg-dexopt=speed-profile \
    pm.dexopt.first-boot=verify \
    pm.dexopt.boot-after-ota=verify \
    pm.dexopt.boot=verify

# Boot
PRODUCT_PROPERTY_OVERRIDES += \
    sys.vendor.shutdown.waittime=500 \
    ro.build.shutdown_timeout=0

# Battery
PRODUCT_PROPERTY_OVERRIDES += \
    ro.config.small_battery=true

# TextClassifier
PRODUCT_COPY_FILES += \
    external/libtextclassifier/native/models/textclassifier.ru.model:$(TARGET_COPY_OUT_SYSTEM)/etc/textclassifier/textclassifier.ru.model \
    external/libtextclassifier/native/models/textclassifier.fr.model:$(TARGET_COPY_OUT_SYSTEM)/etc/textclassifier/textclassifier.fr.model \
    external/libtextclassifier/native/models/textclassifier.pl.model:$(TARGET_COPY_OUT_SYSTEM)/etc/textclassifier/textclassifier.pl.model

# Properties
PRODUCT_PROPERTY_OVERRIDES += \
    keyguard.no_require_sim=true \
    persist.data.qmi.adb_logmask=0 \
    ro.telephony.call_ring.multiple=false \
    ro.use_data_netmgrd=true \
    persist.radio.multisim.config=dsds \
    persist.radio.ecc_hard_count=1 \
    vendor.rild.libpath=/vendor/lib/libril-qc-qmi-1.so \
    ro.telephony.default_network=9,1 \
    ro.telephony.use_old_mnc_mcc_format=true \
    persist.vendor.radio.add_power_save=1 \
    persist.vendor.radio.aosp_usr_pref_sel=true \
    persist.data.mode=concurrent \
    persist.vendor.radio.qcril_uim_vcc_feature=1 \
    persist.vendor.radio.0x9e_not_callname=1 \
    persist.vendor.radio.custom_ecc=1 \
    persist.vendor.radio.data_con_rprt=1 \
    persist.vendor.radio.dfr_mode_set=1 \
    persist.vendor.radio.eri64_as_home=1 \
    persist.vendor.radio.force_get_pref=1 \
    persist.vendor.radio.is_wps_enabled=true \
    persist.radio.msgtunnel.start=true \
    persist.vendor.radio.mt_sms_ack=30 \
    persist.vendor.radio.no_wait_for_card=1 \
    persist.vendor.radio.oem_ind_to_both=false \
    persist.vendor.radio.rat_on=combine \
    persist.vendor.radio.relay_oprt_change=1 \
    persist.vendor.radio.sib16_support=1 \
    persist.vendor.radio.snapshot_timer=22 \
    persist.vendor.radio.snapshot_enabled=1 \
    persist.vendor.radio.apm_sim_not_pwdn=1 \
    persist.rmnet.data.enable=true \
    persist.data.wda.enable=true \
    persist.data.df.dl_mode=5 \
    persist.data.df.ul_mode=5 \
    persist.data.df.agg.dl_pkt=10 \
    persist.data.df.agg.dl_size=4096 \
    persist.data.df.mux_count=8 \
    persist.data.df.iwlan_mux=9 \
    persist.data.df.dev_name=rmnet_usb0 \
    persist.sys.fflag.override.settings_network_and_internet_v2=true \
    persist.data.netmgrd.qos.enable=true \
    fw.max_users=1

# RIL
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/data/netmgr_config.xml:$(TARGET_COPY_OUT_VENDOR)/etc/data/netmgr_config.xml \
    $(LOCAL_PATH)/configs/data/qmi_config.xml:$(TARGET_COPY_OUT_VENDOR)/etc/data/qmi_config.xml \
    $(LOCAL_PATH)/configs/data/dsi_config.xml:$(TARGET_COPY_OUT_VENDOR)/etc/data/dsi_config.xml

# Seccomp
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/seccomp/mediacodec-seccomp.policy:$(TARGET_COPY_OUT_VENDOR)/etc/seccomp_policy/mediacodec.policy \
    $(LOCAL_PATH)/seccomp/mediaextractor-seccomp.policy:$(TARGET_COPY_OUT_VENDOR)/etc/seccomp_policy/mediaextractor.policy \
    $(LOCAL_PATH)/seccomp/mediaswcodec-seccomp.policy:$(TARGET_COPY_OUT_VENDOR)/etc/seccomp_policy/mediaswcodec.policy \
    $(LOCAL_PATH)/seccomp/gnss@2.0-base.policy:$(TARGET_COPY_OUT_VENDOR)/etc/seccomp_policy/gnss@2.0-base.policy \
    $(LOCAL_PATH)/seccomp/gnss@2.0-xtwifi-client.policy:$(TARGET_COPY_OUT_VENDOR)/etc/seccomp_policy/gnss@2.0-xtwifi-client.policy

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
    android.hardware.sensors@1.0-impl.msm8916 \
    android.hardware.sensors@1.0-service.msm8916 \
    sensors.msm8916

# Sensors
PRODUCT_COPY_FILES += \
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


# The target has no boot jars to check
SKIP_BOOT_JARS_CHECK := true

# Since ($SRC_TARGET)/product/generic.mk is included instead of full_base.mk the device config also need to
#  pick up the default Android Platform product locale list
$(call inherit-product, $(SRC_TARGET_DIR)/product/languages_default.mk)

# Override build/make/target/product/emulated_storage.mk to disable casefold
PRODUCT_QUOTA_PROJID := 1
PRODUCT_FS_CASEFOLD := 0
PRODUCT_VENDOR_PROPERTIES += \
    external_storage.projid.enabled=1 \
    external_storage.casefold.enabled=0 \
    external_storage.sdcardfs.enabled=0

# APN(s)
PRODUCT_COPY_FILES += vendor/lineage/prebuilt/common/etc/apns-conf.xml:system/etc/apns-conf.xml
