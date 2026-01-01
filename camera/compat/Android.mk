COMPAT_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PATH := $(COMPAT_PATH)

LOCAL_C_INCLUDES := \
    system/media/camera/include

LOCAL_SRC_FILES := \
    CameraParameters.cpp

LOCAL_SHARED_LIBRARIES := liblog libhardware libutils libcutils libdl libsync

LOCAL_MODULE := libcamera_compat
LOCAL_MODULE_TAGS := optional
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
