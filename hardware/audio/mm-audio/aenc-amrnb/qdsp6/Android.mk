ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

libOmxAmrEnc-def := -g -O3
libOmxAmrEnc-def += -DQC_MODIFIED
libOmxAmrEnc-def += -D_ANDROID_
libOmxAmrEnc-def += -D_ENABLE_QC_MSG_LOG_
libOmxAmrEnc-def += -DVERBOSE
libOmxAmrEnc-def += -D_DEBUG
libOmxAmrEnc-def += -Wconversion
libOmxAmrEnc-def += -DAUDIOV2
libOmxAmrEnc-def += -Wno-sign-conversion -Wno-self-assign -Wno-format -Wno-macro-redefined

# ---------------------------------------------------------------------------------
#             Make the Shared library (libOmxAmrEnc)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

libOmxAmrEnc-inc       := $(LOCAL_PATH)/inc
libOmxAmrEnc-inc       += $(TARGET_OUT_HEADERS)/mm-core/omxcore

LOCAL_MODULE            := libOmxAmrEnc
LOCAL_MODULE_TAGS       := optional
LOCAL_VENDOR_MODULE     := true
LOCAL_CFLAGS            := $(libOmxAmrEnc-def)
LOCAL_C_INCLUDES        := $(libOmxAmrEnc-inc)
LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libutils liblog

LOCAL_SRC_FILES         := src/aenc_svr.c
LOCAL_SRC_FILES         += src/omx_amr_aenc.cpp

LOCAL_HEADER_LIBRARIES := generated_kernel_headers media_headers

include $(BUILD_SHARED_LIBRARY)

endif

# ---------------------------------------------------------------------------------
#                     END
# ---------------------------------------------------------------------------------

