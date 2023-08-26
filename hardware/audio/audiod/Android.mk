LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	audiod_main.cpp \
	AudioDaemon.cpp \

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder \
	libmedia \
        liblog

LOCAL_HEADER_LIBRARIES := generated_kernel_headers

LOCAL_MODULE:= audiod

#include $(BUILD_EXECUTABLE)
