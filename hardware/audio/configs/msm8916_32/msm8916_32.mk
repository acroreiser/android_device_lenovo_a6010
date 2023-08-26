#AUDIO_FEATURE_FLAGS
BOARD_USES_ALSA_AUDIO := true
BOARD_SUPPORTS_SOUND_TRIGGER := true
BOARD_USES_SRS_TRUEMEDIA := false
USE_CUSTOM_AUDIO_POLICY := 0

ifneq ($(TARGET_USES_AOSP_FOR_AUDIO), true)
AUDIO_FEATURE_ENABLED_COMPRESS_VOIP := true
AUDIO_FEATURE_ENABLED_EXTN_FORMATS := true
AUDIO_FEATURE_ENABLED_EXTN_FLAC_DECODER := true
AUDIO_FEATURE_ENABLED_EXTN_RESAMPLER := true
AUDIO_FEATURE_ENABLED_FM_POWER_OPT := true
AUDIO_FEATURE_ENABLED_PROXY_DEVICE := true
AUDIO_FEATURE_ENABLED_VOICE_CONCURRENCY := true
AUDIO_FEATURE_ENABLED_DS2_DOLBY_DAP := false
MM_AUDIO_ENABLED_SAFX := true
DOLBY_ENABLE := false
AUDIO_FEATURE_ENABLED_SSR := true
AUDIO_FEATURE_ENABLED_PCM_OFFLOAD := true
AUDIO_FEATURE_ENABLED_PCM_OFFLOAD_24 := true
AUDIO_FEATURE_ENABLED_FLAC_OFFLOAD := true
AUDIO_FEATURE_ENABLED_VORBIS_OFFLOAD := true
AUDIO_FEATURE_ENABLED_WMA_OFFLOAD := true
AUDIO_FEATURE_ENABLED_ALAC_OFFLOAD := true
AUDIO_FEATURE_ENABLED_APE_OFFLOAD := true
AUDIO_FEATURE_ENABLED_AAC_ADTS_OFFLOAD := true
endif

AUDIO_USE_LL_AS_PRIMARY_OUTPUT := true
AUDIO_FEATURE_ENABLED_VBAT_MONITOR := true
AUDIO_FEATURE_ENABLED_ANC_HEADSET := true
#AUDIO_FEATURE_ENABLED_COMPRESS_CAPTURE := true
AUDIO_FEATURE_ENABLED_FLUENCE := true
AUDIO_FEATURE_ENABLED_HFP := true
#AUDIO_FEATURE_ENABLED_INCALL_MUSIC := true
AUDIO_FEATURE_ENABLED_MULTI_VOICE_SESSIONS := true
AUDIO_FEATURE_ENABLED_SPKR_PROTECTION := true
#AUDIO_FEATURE_ENABLED_MULTIPLE_TUNNEL := true
AUDIO_FEATURE_ENABLED_KPI_OPTIMIZE := true
AUDIO_FEATURE_ENABLED_NT_PAUSE_TIMEOUT := true
MM_AUDIO_ENABLED_FTM := true
AUDIO_FEATURE_ENABLED_ACDB_LICENSE := true
TARGET_USES_QCOM_MM_AUDIO := true
##AUDIO_FEATURE_ENABLED_HW_ACCELERATED_EFFECTS := true
DTS_CODEC_M_ := true
##AUDIO_FEATURE_ENABLED_DTS_EAGLE := true
AUDIO_FEATURE_ENABLED_SOURCE_TRACKING := true
AUDIO_FEATURE_ENABLED_AUDIOSPHERE := true
##AUDIO_FEATURE_ENABLED_HDMI_SPK := true
AUDIO_FEATURE_ENABLED_HDMI_SPK := true
#AUDIO_FEATURE_ENABLED_HDMI_EDID := true
AUDIO_FEATURE_ENABLED_EXT_HDMI := true
#
###not supported feature
##AUDIO_FEATURE_ENABLED_LISTEN := true
##DOLBY_DAP := true
##DOLBY_DDP := true
##DOLBY_UDC := true
##DOLBY_UDC_MULTICHANNEL := true
##DOLBY_UDC_STREAMING_HLS := true
##AUDIO_FEATURE_ENABLED_DEV_ARBI := true
##AUDIO_FEATURE_ENABLED_CUSTOMSTEREO := true
##AUDIO_FEATURE_FLAGS
#
#Audio Specific device overlays
DEVICE_PACKAGE_OVERLAYS += hardware/qcom/audio/configs/common/overlay
#enable software decoders for ALAC and APE
PRODUCT_PROPERTY_OVERRIDES += \
use.qti.sw.alac.decoder=true
PRODUCT_PROPERTY_OVERRIDES += \
use.qti.sw.ape.decoder=true

USE_XML_AUDIO_POLICY_CONF := 1

# Audio configuration file
ifeq ($(TARGET_USES_AOSP), true)
PRODUCT_COPY_FILES += \
    device/qcom/common/media/audio_policy.conf:system/etc/audio_policy.conf
else
PRODUCT_COPY_FILES += \
    hardware/qcom/audio/configs/msm8916_32/audio_policy.conf:system/etc/audio_policy.conf
endif

PRODUCT_COPY_FILES += \
    hardware/qcom/audio/configs/msm8916_32/audio_output_policy.conf:system/vendor/etc/audio_output_policy.conf \
    hardware/qcom/audio/configs/msm8916_32/audio_effects.conf:system/vendor/etc/audio_effects.conf \
    hardware/qcom/audio/configs/msm8916_32/mixer_paths_mtp.xml:system/etc/mixer_paths_mtp.xml \
    hardware/qcom/audio/configs/msm8916_32/mixer_paths_qrd_skuh.xml:system/etc/mixer_paths_qrd_skuh.xml \
    hardware/qcom/audio/configs/msm8916_32/mixer_paths_qrd_skui.xml:system/etc/mixer_paths_qrd_skui.xml \
    hardware/qcom/audio/configs/msm8916_32/mixer_paths_qrd_skuhf.xml:system/etc/mixer_paths_qrd_skuhf.xml \
    hardware/qcom/audio/configs/msm8916_32/mixer_paths_wcd9306.xml:system/etc/mixer_paths_wcd9306.xml \
    hardware/qcom/audio/configs/msm8916_32/mixer_paths_skuk.xml:system/etc/mixer_paths_skuk.xml \
    hardware/qcom/audio/configs/msm8916_32/mixer_paths_qrd_skum.xml:system/etc/mixer_paths_qrd_skum.xml \
    hardware/qcom/audio/configs/msm8916_32/mixer_paths_qrd_skun_cajon.xml:system/etc/mixer_paths_qrd_skun_cajon.xml \
    hardware/qcom/audio/configs/msm8916_32/mixer_paths.xml:system/etc/mixer_paths.xml \
    hardware/qcom/audio/configs/msm8916_32/sound_trigger_mixer_paths.xml:system/etc/sound_trigger_mixer_paths.xml \
    hardware/qcom/audio/configs/msm8916_32/sound_trigger_mixer_paths_wcd9306.xml:system/etc/sound_trigger_mixer_paths_wcd9306.xml \
    hardware/qcom/audio/configs/msm8916_32/sound_trigger_mixer_paths_wcd9330.xml:system/etc/sound_trigger_mixer_paths_wcd9330.xml \
    hardware/qcom/audio/configs/msm8916_32/sound_trigger_platform_info.xml:system/etc/sound_trigger_platform_info.xml \
    hardware/qcom/audio/configs/msm8916_32/mixer_paths_wcd9330.xml:system/etc/mixer_paths_wcd9330.xml \
    hardware/qcom/audio/configs/msm8916_32/mixer_paths_wcd9335.xml:system/etc/mixer_paths_wcd9335.xml \
    hardware/qcom/audio/configs/msm8916_32/mixer_paths_wcd9326.xml:system/etc/mixer_paths_wcd9326.xml \
    hardware/qcom/audio/configs/msm8916_32/mixer_paths_qrd_skun.xml:system/etc/mixer_paths_qrd_skun.xml \
    hardware/qcom/audio/configs/msm8916_32/audio_platform_info.xml:system/etc/audio_platform_info.xml \
    hardware/qcom/audio/configs/msm8916_32/audio_platform_info_extcodec.xml:system/etc/audio_platform_info_extcodec.xml

#XML Audio configuration files
ifeq ($(USE_XML_AUDIO_POLICY_CONF), 1)
ifeq ($(TARGET_USES_AOSP_FOR_AUDIO), true)
PRODUCT_COPY_FILES += \
    $(TOPDIR)hardware/qcom/audio/configs/common/audio_policy_configuration.xml:/system/etc/audio_policy_configuration.xml
else
PRODUCT_COPY_FILES += \
    $(TOPDIR)hardware/qcom/audio/configs/msm8916_32/audio_policy_configuration.xml:system/etc/audio_policy_configuration.xml
endif
PRODUCT_COPY_FILES += \
    $(TOPDIR)frameworks/av/services/audiopolicy/config/a2dp_audio_policy_configuration.xml:/system/etc/a2dp_audio_policy_configuration.xml \
    $(TOPDIR)frameworks/av/services/audiopolicy/config/audio_policy_volumes.xml:/system/etc/audio_policy_volumes.xml \
    $(TOPDIR)frameworks/av/services/audiopolicy/config/default_volume_tables.xml:/system/etc/default_volume_tables.xml \
    $(TOPDIR)frameworks/av/services/audiopolicy/config/r_submix_audio_policy_configuration.xml:/system/etc/r_submix_audio_policy_configuration.xml \
    $(TOPDIR)frameworks/av/services/audiopolicy/config/usb_audio_policy_configuration.xml:/system/etc/usb_audio_policy_configuration.xml
endif

PRODUCT_PACKAGES += \
    libqcomvisualizer \
    libqcompostprocbundle \
    libqcomvoiceprocessing

# Reduce client buffer size for fast audio output tracks
PRODUCT_PROPERTY_OVERRIDES += \
     af.fast_track_multiplier=1

# Low latency audio buffer size in frames
PRODUCT_PROPERTY_OVERRIDES += \
    audio_hal.period_size=192

#split  a2dp
PRODUCT_PROPERTY_OVERRIDES += \
persist.vendor.bt.enable.splita2dp=false
