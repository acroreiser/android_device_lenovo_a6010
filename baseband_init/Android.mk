LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES := \
    system/core/base/include \
    system/core/init \
    external/selinux/libselinux/include
LOCAL_CFLAGS := -Wall
LOCAL_CPP_STD := c++17
LOCAL_SRC_FILES := init_msm8916.cpp
LOCAL_MODULE := libinit_msm8916
LOCAL_STATIC_LIBRARIES := libbase libselinux

include $(BUILD_STATIC_LIBRARY)