LOCAL_PATH:= $(call my-dir)

qcom_post_proc_common_cflags := \
    -O2 -fvisibility=hidden \
    -Wall -Werror \
    -Wno-unused-function \
    -Wno-unused-variable \
    -Wno-sign-compare -Wno-format

include $(CLEAR_VARS)

ifneq ($(strip $(AUDIO_FEATURE_ENABLED_PROXY_DEVICE)),false)
    LOCAL_CFLAGS += -DAFE_PROXY_ENABLED
endif

LOCAL_SRC_FILES:= \
	bundle.c \
	equalizer.c \
	bass_boost.c \
	virtualizer.c \
	reverb.c \
	effect_api.c \
	effect_util.c

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_HW_ACCELERATED_EFFECTS)),true)
    LOCAL_CFLAGS += -DHW_ACCELERATED_EFFECTS
    LOCAL_SRC_FILES += hw_accelerator.c
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_AUDIOSPHERE)),true)
    LOCAL_CFLAGS += -DAUDIOSPHERE_ENABLED
    LOCAL_SRC_FILES += asphere.c
endif

LOCAL_CFLAGS += $(qcom_post_proc_common_cflags)

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_DTS_EAGLE)), true)
    LOCAL_CFLAGS += -DDTS_EAGLE
endif

ifneq ($(filter msm8916,$(TARGET_BOARD_PLATFORM)),)
    LOCAL_CFLAGS += -DPLATFORM_MSM8916
endif

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	liblog \
	libtinyalsa \
	libdl

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_RELATIVE_PATH := soundfx
LOCAL_MODULE:= libqcompostprocbundle
LOCAL_VENDOR_MODULE := true

LOCAL_HEADER_LIBRARIES := generated_kernel_headers

LOCAL_C_INCLUDES := \
	external/tinyalsa/include \
	$(call include-path-for, audio-effects)

include $(BUILD_SHARED_LIBRARY)

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_HW_ACCELERATED_EFFECTS)),true)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := EffectsHwAcc.cpp

LOCAL_C_INCLUDES := \
    $(call include-path-for, audio-effects)

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libeffects

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS += $(qcom_post_proc_common_cflags)

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_DTS_EAGLE)), true)
LOCAL_CFLAGS += -DHW_ACC_HPX
endif

LOCAL_MODULE:= libhwacceffectswrapper
LOCAL_VENDOR_MODULE := true

include $(BUILD_STATIC_LIBRARY)
endif
