ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

libOmxQcelp13Enc-def := -g -O3
libOmxQcelp13Enc-def += -DQC_MODIFIED
libOmxQcelp13Enc-def += -D_ANDROID_
libOmxQcelp13Enc-def += -D_ENABLE_QC_MSG_LOG_
libOmxQcelp13Enc-def += -DVERBOSE
libOmxQcelp13Enc-def += -D_DEBUG
libOmxQcelp13Enc-def += -Wconversion
libOmxQcelp13Enc-def += -DAUDIOV2
libOmxQcelp13Enc-def += -Wno-sign-conversion -Wno-self-assign -Wno-format -Wno-macro-redefined

# ---------------------------------------------------------------------------------
#             Make the Shared library (libOmxQcelp13Enc)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

libOmxQcelp13Enc-inc       := $(LOCAL_PATH)/inc
libOmxQcelp13Enc-inc       += $(TARGET_OUT_HEADERS)/mm-core/omxcore

LOCAL_MODULE            := libOmxQcelp13Enc
LOCAL_MODULE_TAGS       := optional
LOCAL_VENDOR_MODULE     := true
LOCAL_CFLAGS            := $(libOmxQcelp13Enc-def)
LOCAL_C_INCLUDES        := $(libOmxQcelp13Enc-inc)
LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libutils liblog

LOCAL_SRC_FILES         := src/aenc_svr.c
LOCAL_SRC_FILES         += src/omx_qcelp13_aenc.cpp

LOCAL_HEADER_LIBRARIES := generated_kernel_headers media_headers

include $(BUILD_SHARED_LIBRARY)

endif

# ---------------------------------------------------------------------------------
#                     END
# ---------------------------------------------------------------------------------

