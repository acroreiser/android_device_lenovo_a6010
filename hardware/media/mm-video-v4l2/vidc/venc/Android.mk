LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
# 				Common definitons
# ---------------------------------------------------------------------------------

libmm-venc-def := -g -O3 -Dlrintf=_ffix_r
libmm-venc-def += -D__align=__alignx
libmm-venc-def += -D__alignx\(x\)=__attribute__\(\(__aligned__\(x\)\)\)
libmm-venc-def += -DT_ARM
libmm-venc-def += -Dinline=__inline
libmm-venc-def += -D_ANDROID_
libmm-venc-def += -UENABLE_DEBUG_LOW
libmm-venc-def += -UENABLE_DEBUG_HIGH
libmm-venc-def += -DENABLE_DEBUG_ERROR
libmm-venc-def += -UINPUT_BUFFER_LOG
libmm-venc-def += -UOUTPUT_BUFFER_LOG
libmm-venc-def += -USINGLE_ENCODER_INSTANCE
libmm-venc-def += -Werror
libmm-venc-def += -D_ANDROID_ICS_
libmm-venc-def += -D_MSM8974_

TARGETS_THAT_USE_FLAG_MSM8226 := msm8226 msm8916
ifneq "$(wildcard $(QCPATH) )" ""
# relevant libraries are not built for OSS builds as QCPATH is null in OSS builds.
ifneq ($(TARGET_SUPPORTS_WEARABLES),true)
TARGETS_THAT_NEED_SW_VENC_MPEG4 := msm8909
endif
TARGETS_THAT_NEED_SW_VENC_HEVC := msm8992
endif

ifeq ($(TARGET_BOARD_PLATFORM),msm8610)
libmm-venc-def += -DMAX_RES_720P
libmm-venc-def += -D_MSM8610_
else
ifeq ($(TARGET_BOARD_PLATFORM),msm8226)
libmm-venc-def += -DMAX_RES_1080P
else
libmm-venc-def += -DMAX_RES_1080P
libmm-venc-def += -DMAX_RES_1080P_EBI
endif
endif

ifeq ($(call is-board-platform-in-list, $(TARGETS_THAT_USE_FLAG_MSM8226)),true)
libmm-venc-def += -D_MSM8226_
endif

ifeq ($(TARGET_BOARD_PLATFORM),msm8909)
libmm-venc-def += -D_MSM8909_
endif

ifeq ($(TARGET_USES_ION),true)
libmm-venc-def += -DUSE_ION
endif

ifeq ($(TARGET_USES_MEDIA_EXTENSIONS),true)
libmm-venc-def += -DUSE_NATIVE_HANDLE_SOURCE
endif

ifeq ($(TARGET_USES_MEDIA_EXTENSIONS),true)
libmm-venc-def += -DSUPPORT_CONFIG_INTRA_REFRESH
endif

# Common Includes
libmm-venc-inc      := $(LOCAL_PATH)/inc
libmm-venc-inc      += $(LOCAL_PATH)/../../../mm-video-v4l2/vidc/common/inc
libmm-venc-inc      += $(LOCAL_PATH)/../../../mm-core/inc
libmm-venc-inc      += $(LOCAL_PATH)/../../../libstagefrighthw
libmm-venc-inc      += $(TARGET_OUT_HEADERS)/qcom/display
libmm-venc-inc      += $(TARGET_OUT_HEADERS)/adreno
libmm-venc-inc      += $(LOCAL_PATH)/../../../libc2dcolorconvert
libmm-venc-inc      += frameworks/av/include/media/stagefright


# ---------------------------------------------------------------------------------
# 			Make the Shared library (libOmxVenc)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

LOCAL_HEADER_LIBRARIES := \
        display_headers \
        generated_kernel_headers \
        media_plugin_headers \
        libnativebase_headers \
        libcutils_headers \
        libutils_headers

LOCAL_MODULE                    := libOmxVenc
LOCAL_MODULE_TAGS               := optional
LOCAL_VENDOR_MODULE             := true
LOCAL_CFLAGS                    := $(libmm-venc-def) -Wno-misleading-indentation
LOCAL_C_INCLUDES                := $(libmm-venc-inc)

LOCAL_PRELINK_MODULE      := false
LOCAL_SHARED_LIBRARIES    := liblog libutils libcutils \
                             libc2dcolorconvert libdl libgui_vendor
LOCAL_SHARED_LIBRARIES += libqdMetaData
LOCAL_STATIC_LIBRARIES    := libOmxVidcCommon

LOCAL_SRC_FILES   := src/omx_video_base.cpp
LOCAL_SRC_FILES   += src/omx_video_encoder.cpp
LOCAL_SRC_FILES   += src/video_encoder_device_v4l2.cpp

include $(BUILD_SHARED_LIBRARY)

ifeq ($(BOARD_USES_QCOM_HARDWARE),true)
ifeq ($(call is-board-platform-in-list, $(TARGETS_THAT_NEED_SW_VENC_MPEG4)),true)
# ---------------------------------------------------------------------------------
# 			Make the Shared library (libOmxSwVencMpeg4)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

libmm-venc-inc      += $(TARGET_OUT_HEADERS)/mm-video/swvenc

LOCAL_HEADER_LIBRARIES := display_headers generated_kernel_headers

LOCAL_MODULE                    := libOmxSwVencMpeg4

LOCAL_MODULE_TAGS               := optional
LOCAL_VENDOR_MODULE             := true
LOCAL_CFLAGS                    := $(libmm-venc-def)
LOCAL_C_INCLUDES                := $(libmm-venc-inc)

LOCAL_PRELINK_MODULE      := false
LOCAL_SHARED_LIBRARIES    := liblog libutils libcutils \
                             libc2dcolorconvert libdl libgui_vendor
LOCAL_SHARED_LIBRARIES    += libMpeg4SwEncoder
LOCAL_STATIC_LIBRARIES    := libOmxVidcCommon

LOCAL_SRC_FILES   := src/omx_video_base.cpp
LOCAL_SRC_FILES   += src/omx_swvenc_mpeg4.cpp

include $(BUILD_SHARED_LIBRARY)
endif
endif

ifeq ($(call is-board-platform-in-list, $(TARGETS_THAT_NEED_SW_VENC_HEVC)),true)
# ---------------------------------------------------------------------------------
#                            Make the Shared library (libOmxSwVenc)
# ---------------------------------------------------------------------------------
include $(CLEAR_VARS)

libmm-venc-inc      += $(TARGET_OUT_HEADERS)/mm-video/swVenc

LOCAL_HEADER_LIBRARIES := display_headers generated_kernel_headers

LOCAL_MODULE                    := libOmxSwVencHevc
LOCAL_MODULE_TAGS               := optional
LOCAL_VENDOR_MODULE             := true
LOCAL_CFLAGS                    := $(libmm-venc-def)
LOCAL_C_INCLUDES                := $(libmm-venc-inc)

LOCAL_PRELINK_MODULE      := false
LOCAL_SHARED_LIBRARIES    := liblog libutils libbinder libcutils \
                             libc2dcolorconvert libdl libgui_vendor
LOCAL_SHARED_LIBRARIES    += libHevcSwEncoder
LOCAL_STATIC_LIBRARIES    := libOmxVidcCommon

LOCAL_SRC_FILES   := src/omx_video_base.cpp
LOCAL_SRC_FILES   += src/omx_swvenc_hevc.cpp

include $(BUILD_SHARED_LIBRARY)
endif

# ---------------------------------------------------------------------------------
# 					END
# ---------------------------------------------------------------------------------
