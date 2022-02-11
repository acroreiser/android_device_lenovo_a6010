LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := msm_irqbalance.c

LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_SHARED_LIBRARIES += libutils

LOCAL_MODULE := msm_irqbalance

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS += \
	-DUSE_ANDROID_LOG \

include $(BUILD_EXECUTABLE)