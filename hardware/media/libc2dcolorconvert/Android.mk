LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(TARGET_USES_ALIGNED_YCBCR420P),true)
LOCAL_CFLAGS += -DUSE_ALIGNED_YCBCR420P
endif

LOCAL_SRC_FILES := \
        C2DColorConverter.cpp

LOCAL_C_INCLUDES := \
    $(TARGET_OUT_HEADERS)/qcom/display

LOCAL_SHARED_LIBRARIES := liblog libdl

LOCAL_HEADER_LIBRARIES := \
        display_headers \
        generated_kernel_headers \
        libutils_headers \
        libhardware_headers

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libc2dcolorconvert

LOCAL_VENDOR_MODULE := true

include $(BUILD_SHARED_LIBRARY)
