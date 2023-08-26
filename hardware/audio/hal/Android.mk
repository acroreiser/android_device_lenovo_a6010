ifeq ($(strip $(BOARD_USES_ALSA_AUDIO)),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

AUDIO_PLATFORM := $(TARGET_BOARD_PLATFORM)

ifneq ($(filter msm8974 msm8226 msm8610 apq8084 msm8994 msm8992 msm8996,$(TARGET_BOARD_PLATFORM)),)
  # B-family platform uses msm8974 code base
  AUDIO_PLATFORM = msm8974
  MULTIPLE_HW_VARIANTS_ENABLED := true
ifneq ($(filter msm8610,$(TARGET_BOARD_PLATFORM)),)
  LOCAL_CFLAGS := -DPLATFORM_MSM8610
endif
ifneq ($(filter msm8226,$(TARGET_BOARD_PLATFORM)),)
  LOCAL_CFLAGS := -DPLATFORM_MSM8x26
endif
ifneq ($(filter apq8084,$(TARGET_BOARD_PLATFORM)),)
  LOCAL_CFLAGS := -DPLATFORM_APQ8084
endif
ifneq ($(filter msm8994,$(TARGET_BOARD_PLATFORM)),)
  LOCAL_CFLAGS := -DPLATFORM_MSM8994
endif
ifneq ($(filter msm8992,$(TARGET_BOARD_PLATFORM)),)
  LOCAL_CFLAGS := -DPLATFORM_MSM8994
endif
ifneq ($(filter msm8996,$(TARGET_BOARD_PLATFORM)),)
  LOCAL_CFLAGS := -DPLATFORM_MSM8996
endif
endif

ifneq ($(filter msm8916 msm8909 msm8952,$(TARGET_BOARD_PLATFORM)),)
  AUDIO_PLATFORM = msm8916
  MULTIPLE_HW_VARIANTS_ENABLED := true
  LOCAL_CFLAGS := -DPLATFORM_MSM8916
ifneq ($(filter msm8909,$(TARGET_BOARD_PLATFORM)),)
  LOCAL_CFLAGS := -DPLATFORM_MSM8909
endif
endif

LOCAL_SRC_FILES := \
	audio_hw.c \
	voice.c \
	platform_info.c \
	$(AUDIO_PLATFORM)/platform.c \
        acdb.c

LOCAL_SRC_FILES += audio_extn/audio_extn.c \
                   audio_extn/utils.c
LOCAL_HEADER_LIBRARIES := generated_kernel_headers audio_headers
LOCAL_CFLAGS += -DUSE_VENDOR_EXTN

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_HDMI_EDID)),true)
    LOCAL_SRC_FILES += edid.c
    LOCAL_CFLAGS += -DHDMI_EDID
endif

ifeq ($(strip $(AUDIO_USE_LL_AS_PRIMARY_OUTPUT)),true)
    LOCAL_CFLAGS += -DUSE_LL_AS_PRIMARY_OUTPUT
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_PCM_OFFLOAD)),true)
    LOCAL_CFLAGS += -DPCM_OFFLOAD_ENABLED
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_ANC_HEADSET)),true)
    LOCAL_CFLAGS += -DANC_HEADSET_ENABLED
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_VBAT_MONITOR)),true)
    LOCAL_CFLAGS += -DVBAT_MONITOR_ENABLED
endif

ifneq ($(strip $(AUDIO_FEATURE_ENABLED_FLUENCE)),false)
    LOCAL_CFLAGS += -DFLUENCE_ENABLED
endif

ifneq ($(strip $(AUDIO_FEATURE_ENABLED_PROXY_DEVICE)),false)
    LOCAL_CFLAGS += -DAFE_PROXY_ENABLED
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_KPI_OPTIMIZE)),true)
    LOCAL_CFLAGS += -DKPI_OPTIMIZE_ENABLED
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_FM_POWER_OPT)),true)
    LOCAL_CFLAGS += -DFM_POWER_OPT
    LOCAL_SRC_FILES += audio_extn/fm.c
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_USBAUDIO)),true)
    LOCAL_CFLAGS += -DUSB_HEADSET_ENABLED
    LOCAL_SRC_FILES += audio_extn/usb.c
endif

ifneq ($(strip $(AUDIO_FEATURE_ENABLED_HFP)),false)
    LOCAL_CFLAGS += -DHFP_ENABLED
    LOCAL_SRC_FILES += audio_extn/hfp.c
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_CUSTOMSTEREO)),true)
    LOCAL_CFLAGS += -DCUSTOM_STEREO_ENABLED
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_SSR)),true)
    LOCAL_CFLAGS += -DSSR_ENABLED
    LOCAL_SRC_FILES += audio_extn/ssr.c
    LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-audio/surround_sound_3mic/
    LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc/
endif

ifeq ($(strip $(AUDIO_FEATURE_HUAWEI_SOUND_PARAM_PATH)),true)
    LOCAL_CFLAGS += -DHUAWEI_SOUND_PARAM_PATH
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_MULTI_VOICE_SESSIONS)),true)
    LOCAL_CFLAGS += -DMULTI_VOICE_SESSION_ENABLED
    LOCAL_SRC_FILES += voice_extn/voice_extn.c

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_INCALL_MUSIC)),true)
    LOCAL_CFLAGS += -DINCALL_MUSIC_ENABLED
endif
ifeq ($(strip $(AUDIO_FEATURE_HTC_DUAL_SIM)),true)
    LOCAL_CFLAGS += -DHTC_DUAL_SIM
    LOCAL_SRC_FILES += voice_extn/msim_voice_extn.c
endif
ifeq ($(strip $(AUDIO_FEATURE_SAMSUNG_DUAL_SIM)),true)
    LOCAL_CFLAGS += -DSAMSUNG_DUAL_SIM
    LOCAL_SRC_FILES += voice_extn/msim_voice_extn.c
endif
ifneq ($(strip $(AUDIO_FEATURE_ENABLED_COMPRESS_VOIP)),false)
    LOCAL_CFLAGS += -DCOMPRESS_VOIP_ENABLED
    LOCAL_SRC_FILES += voice_extn/compress_voip.c
endif

endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_EXTN_FORMATS)),true)
LOCAL_CFLAGS += -DAUDIO_EXTN_FORMATS_ENABLED
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_EXTENDED_COMPRESS_FORMAT)),true)
  LOCAL_CFLAGS += -DENABLE_EXTENDED_COMPRESS_FORMAT
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_SPKR_PROTECTION)),true)
    LOCAL_CFLAGS += -DSPKR_PROT_ENABLED
    LOCAL_SRC_FILES += audio_extn/spkr_protection.c
endif

ifdef MULTIPLE_HW_VARIANTS_ENABLED
  LOCAL_CFLAGS += -DHW_VARIANTS_ENABLED
  LOCAL_SRC_FILES += $(AUDIO_PLATFORM)/hw_info.c
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_COMPRESS_CAPTURE)),true)
    LOCAL_CFLAGS += -DCOMPRESS_CAPTURE_ENABLED
    LOCAL_SRC_FILES += audio_extn/compress_capture.c
endif

ifneq ($(strip $(AUDIO_FEATURE_ENABLED_NEW_SAMPLE_RATE)),false)
    LOCAL_CFLAGS += -DNEW_SAMPLE_RATE_ENABLED
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_DTS_EAGLE)),true)
    LOCAL_CFLAGS += -DDTS_EAGLE
    LOCAL_SRC_FILES += audio_extn/dts_eagle.c
endif

ifeq ($(strip $(DOLBY_DDP)),true)
    LOCAL_CFLAGS += -DDS1_DOLBY_DDP_ENABLED
    LOCAL_SRC_FILES += audio_extn/dolby.c
endif

ifeq ($(strip $(DS1_DOLBY_DAP)),true)
    LOCAL_CFLAGS += -DDS1_DOLBY_DAP_ENABLED
ifneq ($(strip $(DOLBY_DDP)),true)
    LOCAL_SRC_FILES += audio_extn/dolby.c
endif
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_MULTIPLE_TUNNEL)), true)
    LOCAL_CFLAGS += -DMULTIPLE_OFFLOAD_ENABLED
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_EXTN_FLAC_DECODER)),true)
    LOCAL_CFLAGS += -DFLAC_OFFLOAD_ENABLED
    LOCAL_CFLAGS += -DCOMPRESS_METADATA_NEEDED
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_VORBIS_OFFLOAD)),true)
    LOCAL_CFLAGS += -DVORBIS_OFFLOAD_ENABLED
    LOCAL_CFLAGS += -DCOMPRESS_METADATA_NEEDED

endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_WMA_OFFLOAD)),true)
    LOCAL_CFLAGS += -DWMA_OFFLOAD_ENABLED
    LOCAL_CFLAGS += -DCOMPRESS_METADATA_NEEDED
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_ALAC_OFFLOAD)),true)
    LOCAL_CFLAGS += -DALAC_OFFLOAD_ENABLED
    LOCAL_CFLAGS += -DCOMPRESS_METADATA_NEEDED
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_APE_OFFLOAD)),true)
    LOCAL_CFLAGS += -DAPE_OFFLOAD_ENABLED
    LOCAL_CFLAGS += -DCOMPRESS_METADATA_NEEDED
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_PCM_OFFLOAD_24)),true)
       LOCAL_CFLAGS += -DPCM_OFFLOAD_ENABLED_24
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_AAC_ADTS_OFFLOAD)),true)
    LOCAL_CFLAGS += -DAAC_ADTS_OFFLOAD_ENABLED
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_DEV_ARBI)),true)
    LOCAL_CFLAGS += -DDEV_ARBI_ENABLED
    LOCAL_SRC_FILES += audio_extn/dev_arbi.c
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_RECORD_PLAY_CONCURRENCY)),true)
    LOCAL_CFLAGS += -DRECORD_PLAY_CONCURRENCY
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_ACDB_LICENSE)), true)
    LOCAL_CFLAGS += -DDOLBY_ACDB_LICENSE
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_DS2_DOLBY_DAP)),true)
    LOCAL_CFLAGS += -DDS2_DOLBY_DAP_ENABLED
    LOCAL_CFLAGS += -DDS1_DOLBY_DDP_ENABLED
ifneq ($(strip $(DOLBY_DDP)),true)
    ifneq ($(strip $(DS1_DOLBY_DAP)),true)
        LOCAL_SRC_FILES += audio_extn/dolby.c
    endif
endif
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_HDMI_PASSTHROUGH)),true)
    LOCAL_CFLAGS += -DHDMI_PASSTHROUGH_ENABLED
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_SOURCE_TRACKING)),true)
    LOCAL_CFLAGS += -DSOURCE_TRACKING_ENABLED
    LOCAL_SRC_FILES += audio_extn/source_track.c
endif

LOCAL_SHARED_LIBRARIES := \
        liblog \
        libcutils \
        libhardware \
        libtinyalsa \
        libtinycompress \
        libaudioroute \
        libdl \
        libaudioutils \
        libexpat \
        libprocessgroup

LOCAL_C_INCLUDES += \
        external/tinyalsa/include \
        external/tinycompress/include \
        external/expat/lib \
        hardware/libhardware/include \
        system/media/audio_utils/include \
        $(call include-path-for, audio-route) \
        $(call include-path-for, audio-effects) \
        $(LOCAL_PATH)/$(AUDIO_PLATFORM) \
        $(LOCAL_PATH)/audio_extn \
        $(LOCAL_PATH)/voice_extn

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_LISTEN)),true)
    LOCAL_CFLAGS += -DAUDIO_LISTEN_ENABLED
    LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-audio/audio-listen
    LOCAL_SRC_FILES += audio_extn/listen.c
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_EXT_HDMI)),true)
    LOCAL_CFLAGS += -DAUDIO_EXTERNAL_HDMI_ENABLED
    LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-audio/audio-parsers
    LOCAL_SHARED_LIBRARIES += libaudioparsers
endif

ifeq ($(strip $(BOARD_SUPPORTS_SOUND_TRIGGER)),true)
    LOCAL_CFLAGS += -DSOUND_TRIGGER_ENABLED
    LOCAL_CFLAGS += -DSOUND_TRIGGER_PLATFORM_NAME=$(TARGET_BOARD_PLATFORM)
    LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-audio/sound_trigger
    LOCAL_SRC_FILES += audio_extn/soundtrigger.c
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_AUXPCM_BT)),true)
    LOCAL_CFLAGS += -DAUXPCM_BT_ENABLED
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_PM_SUPPORT)),true)
    LOCAL_CFLAGS += -DPM_SUPPORT_ENABLED
    LOCAL_SRC_FILES += audio_extn/pm.c
    LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/libperipheralclient/inc
    LOCAL_SHARED_LIBRARIES += libperipheral_client
endif

ifneq ($(strip $(AUDIO_FEATURE_ENABLED_EXT_AMPLIFIER)),false)
    LOCAL_CFLAGS += -DEXT_AMPLIFIER_ENABLED
    LOCAL_SRC_FILES += audio_extn/audio_amplifier.c
endif

LOCAL_CFLAGS += -Werror

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_SND_MONITOR)), true)
    LOCAL_CFLAGS += -DSND_MONITOR_ENABLED
    LOCAL_SRC_FILES += audio_extn/sndmonitor.c
endif

LOCAL_MODULE := audio.primary.$(TARGET_BOARD_PLATFORM)

LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_MODULE_TAGS := optional

LOCAL_VENDOR_MODULE := true

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := audio_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/audio_extn
include $(BUILD_HEADER_LIBRARY)

endif
