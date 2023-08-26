#AUDIO_FEATURE_FLAGS
BOARD_USES_ALSA_AUDIO := true
BOARD_SUPPORTS_SOUND_TRIGGER := true
BOARD_USES_SRS_TRUEMEDIA := false
USE_CUSTOM_AUDIO_POLICY := 1

ifneq ($(TARGET_USES_AOSP_FOR_AUDIO), true)
AUDIO_FEATURE_ENABLED_COMPRESS_VOIP := true
AUDIO_FEATURE_ENABLED_EXTN_FORMATS := true
AUDIO_FEATURE_ENABLED_EXTN_FLAC_DECODER := true
AUDIO_FEATURE_ENABLED_EXTN_RESAMPLER := true
AUDIO_FEATURE_ENABLED_FM_POWER_OPT := true
AUDIO_FEATURE_ENABLED_PROXY_DEVICE := true
AUDIO_FEATURE_ENABLED_VOICE_CONCURRENCY := true
AUDIO_FEATURE_ENABLED_DS2_DOLBY_DAP := true
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
AUDIO_FEATURE_ENABLED_SND_MONITOR := true
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
vendor.audio.use.sw.alac.decoder=true
PRODUCT_PROPERTY_OVERRIDES += \
vendor.audio.use.sw.ape.decoder=true

USE_XML_AUDIO_POLICY_CONF := 1

# Audio configuration file
ifeq ($(TARGET_USES_AOSP), true)
PRODUCT_COPY_FILES += \
    device/qcom/common/media/audio_policy.conf:system/etc/audio_policy.conf
else
PRODUCT_COPY_FILES += \
    hardware/qcom/audio/configs/msm8952_32/audio_policy.conf:system/etc/audio_policy.conf
endif

PRODUCT_COPY_FILES += \
    hardware/qcom/audio/configs/msm8952_32/audio_output_policy.conf:system/vendor/etc/audio_output_policy.conf \
    hardware/qcom/audio/configs/msm8952_32/audio_effects.conf:system/vendor/etc/audio_effects.conf \
    hardware/qcom/audio/configs/msm8952_32/mixer_paths_mtp.xml:system/etc/mixer_paths_mtp.xml \
    hardware/qcom/audio/configs/msm8952_32/mixer_paths_qrd_skuh.xml:system/etc/mixer_paths_qrd_skuh.xml \
    hardware/qcom/audio/configs/msm8952_32/mixer_paths_qrd_skui.xml:system/etc/mixer_paths_qrd_skui.xml \
    hardware/qcom/audio/configs/msm8952_32/mixer_paths_qrd_skuhf.xml:system/etc/mixer_paths_qrd_skuhf.xml \
    hardware/qcom/audio/configs/msm8952_32/mixer_paths_wcd9306.xml:system/etc/mixer_paths_wcd9306.xml \
    hardware/qcom/audio/configs/msm8952_32/mixer_paths_skuk.xml:system/etc/mixer_paths_skuk.xml \
    hardware/qcom/audio/configs/msm8952_32/mixer_paths_qrd_skum.xml:system/etc/mixer_paths_qrd_skum.xml \
    hardware/qcom/audio/configs/msm8952_32/mixer_paths_qrd_skun_cajon.xml:system/etc/mixer_paths_qrd_skun_cajon.xml \
    hardware/qcom/audio/configs/msm8952_32/mixer_paths_msm8952_polaris.xml:system/etc/mixer_paths_msm8952_polaris.xml \
    hardware/qcom/audio/configs/msm8952_32/mixer_paths.xml:system/etc/mixer_paths.xml \
    hardware/qcom/audio/configs/msm8952_32/sound_trigger_mixer_paths.xml:system/etc/sound_trigger_mixer_paths.xml \
    hardware/qcom/audio/configs/msm8952_32/sound_trigger_mixer_paths_wcd9306.xml:system/etc/sound_trigger_mixer_paths_wcd9306.xml \
    hardware/qcom/audio/configs/msm8952_32/sound_trigger_mixer_paths_wcd9330.xml:system/etc/sound_trigger_mixer_paths_wcd9330.xml \
    hardware/qcom/audio/configs/msm8952_32/sound_trigger_platform_info.xml:system/etc/sound_trigger_platform_info.xml \
    hardware/qcom/audio/configs/msm8952_32/mixer_paths_wcd9330.xml:system/etc/mixer_paths_wcd9330.xml \
    hardware/qcom/audio/configs/msm8952_32/mixer_paths_wcd9335.xml:system/etc/mixer_paths_wcd9335.xml \
    hardware/qcom/audio/configs/msm8952_32/mixer_paths_wcd9326.xml:system/etc/mixer_paths_wcd9326.xml \
    hardware/qcom/audio/configs/msm8952_32/mixer_paths_qrd_skun.xml:system/etc/mixer_paths_qrd_skun.xml \
    hardware/qcom/audio/configs/msm8952_32/audio_platform_info.xml:system/etc/audio_platform_info.xml \
    hardware/qcom/audio/configs/msm8952_32/audio_platform_info_extcodec.xml:system/etc/audio_platform_info_extcodec.xml

#XML Audio configuration files
ifeq ($(USE_XML_AUDIO_POLICY_CONF), 1)
ifeq ($(TARGET_USES_AOSP_FOR_AUDIO), true)
PRODUCT_COPY_FILES += \
    $(TOPDIR)hardware/qcom/audio/configs/common/audio_policy_configuration.xml:/system/etc/audio_policy_configuration.xml
else
PRODUCT_COPY_FILES += \
    $(TOPDIR)hardware/qcom/audio/configs/msm8952_32/audio_policy_configuration.xml:system/etc/audio_policy_configuration.xml
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

#Disable surround sound recording
PRODUCT_PROPERTY_OVERRIDES += \
ro.vendor.audio.sdk.ssr=false

##fluencetype can be "fluence" or "fluencepro" or "none"
PRODUCT_PROPERTY_OVERRIDES += \
ro.vendor.audio.sdk.fluencetype=none\
persist.vendor.audio.fluence.voicecall=true\
persist.vendor.audio.fluence.voicerec=false\
persist.vendor.audio.fluence.speaker=true

#disable tunnel encoding
PRODUCT_PROPERTY_OVERRIDES += \
vendor.audio.tunnel.encode = false

#Buffer size in kbytes for compress offload playback
PRODUCT_PROPERTY_OVERRIDES += \
vendor.audio.offload.buffer.size.kb=64

#Minimum duration for offload playback in secs
PRODUCT_PROPERTY_OVERRIDES += \
audio.offload.min.duration.secs=30

#Enable offload audio video playback by default
PRODUCT_PROPERTY_OVERRIDES += \
audio.offload.video=true

#Enable audio track offload by default
PRODUCT_PROPERTY_OVERRIDES += \
vendor.audio.offload.track.enable=true

#Enable music through deep buffer
PRODUCT_PROPERTY_OVERRIDES += \
audio.deep_buffer.media=true

#enable voice path for PCM VoIP by default
PRODUCT_PROPERTY_OVERRIDES += \
vendor.voice.path.for.pcm.voip=true

#enable downsampling for multi-channel content > 48Khz
PRODUCT_PROPERTY_OVERRIDES += \
vendor.audio.playback.mch.downsample=true

#enable dsp gapless mode by default
PRODUCT_PROPERTY_OVERRIDES += \
vendor.audio.offload.gapless.enabled=true

#Disable Multiple offload sesison
PRODUCT_PROPERTY_OVERRIDES += \
vendor.audio.offload.multiple.enabled=false

#enable pbe effects
PRODUCT_PROPERTY_OVERRIDES += \
vendor.audio.safx.pbe.enabled=true

#property for AudioSphere Post processing
PRODUCT_PROPERTY_OVERRIDES += \
vendor.audio.pp.asphere.enabled=false

#property to enable DS2 dap
PRODUCT_PROPERTY_OVERRIDES += \
vendor.audio.dolby.ds2.enabled=true

#Audio voice concurrency related flags
PRODUCT_PROPERTY_OVERRIDES += \
vendor.voice.playback.conc.disabled=true\
vendor.voice.record.conc.disabled=false\
vendor.voice.voip.conc.disabled=true

#Decides the audio fallback path during voice call,
#deep-buffer and fast are the two
#allowed fallback paths now.
PRODUCT_PROPERTY_OVERRIDES += \
vendor.voice.conc.fallbackpath=deep-buffer

#parser input buffer size(256kb) in byte stream mode
PRODUCT_PROPERTY_OVERRIDES += \
vendor.audio.parser.ip.buffer.size=262144

#Audio VoIP / playback record concurrency flags
PRODUCT_PROPERTY_OVERRIDES += \
vendor.audio.rec.playback.conc.disabled=false

#enable hw aac encoder by default
PRODUCT_PROPERTY_OVERRIDES += \
vendor.audio.hw.aac.encoder=false

#Property to enable FBSP
PRODUCT_PROPERTY_OVERRIDES += \
persist.vendor.audio.speaker.prot.enable=false

# for HIDL related packages
PRODUCT_PACKAGES += \
    android.hardware.audio@2.0-service \
    android.hardware.audio@2.0-impl \
    android.hardware.audio.effect@2.0-impl \
    android.hardware.soundtrigger@2.0-impl
