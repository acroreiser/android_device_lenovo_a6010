ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

libOmxAacEnc-def := -g -O3
libOmxAacEnc-def += -DQC_MODIFIED
libOmxAacEnc-def += -D_ANDROID_
libOmxAacEnc-def += -D_ENABLE_QC_MSG_LOG_
libOmxAacEnc-def += -DVERBOSE
libOmxAacEnc-def += -D_DEBUG
libOmxAacEnc-def += -Wconversion
libOmxAacEnc-def += -DAUDIOV2
libOmxAacEnc-def += -Wno-sign-conversion -Wno-self-assign -Wno-format -Wno-macro-redefined -Wno-sign-compare

# ---------------------------------------------------------------------------------
#             Make the Shared library (libOmxAacEnc)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

libOmxAacEnc-inc       := $(LOCAL_PATH)/inc
libOmxAacEnc-inc       += $(TARGET_OUT_HEADERS)/mm-core/omxcore

LOCAL_MODULE            := libOmxAacEnc
LOCAL_MODULE_TAGS       := optional
LOCAL_VENDOR_MODULE     := true
LOCAL_CFLAGS            := $(libOmxAacEnc-def)
LOCAL_C_INCLUDES        := $(libOmxAacEnc-inc)
LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libutils liblog

LOCAL_SRC_FILES         := src/aenc_svr.c
LOCAL_SRC_FILES         += src/omx_aac_aenc.cpp

LOCAL_HEADER_LIBRARIES := generated_kernel_headers media_headers

include $(BUILD_SHARED_LIBRARY)

endif

# ---------------------------------------------------------------------------------
#                     END
# ---------------------------------------------------------------------------------

