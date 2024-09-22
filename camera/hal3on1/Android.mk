HAL3ON1_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PATH := $(HAL3ON1_PATH)
LOCAL_C_INCLUDES := \
    system/media/camera/include \
    frameworks/native/include \
    external/libyuv/files/include

LOCAL_SRC_FILES := \
    HAL3on1-adapter.cpp

LOCAL_SHARED_LIBRARIES := \
    libhardware \
    liblog \
    libcamera_client \
    libutils \
    libcutils \
    libbase \
    libsensor \
    libhidltransport \
    libnativewindow \
    libgui_vendor \
    libui \
    android.hidl.token@1.0-utils \
    android.hardware.graphics.bufferqueue@1.0 \
    libbinder \
    libjpeg

LOCAL_STATIC_LIBRARIES := \
    libarect \
    libyuv_static

LOCAL_CPP_FLAGS += -DLOG_NDEBUG

LOCAL_HEADER_LIBRARIES := libnativebase_headers
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE := camera.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_TAGS := optional
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
