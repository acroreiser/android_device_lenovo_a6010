OV13850_CHROMATIX_SNAPSHOT_PATH := $(call my-dir)


# ---------------------------------------------------------------------------
#                      Make the shared library (libchromatix_ov13850_snapshot)
# ---------------------------------------------------------------------------


include $(CLEAR_VARS)
LOCAL_PATH := $(OV13850_CHROMATIX_SNAPSHOT_PATH)
LOCAL_MODULE_TAGS := optional


LOCAL_CFLAGS:= -DAMSS_VERSION=$(AMSS_VERSION) \
        $(mmcamera_debug_defines) \
        $(mmcamera_debug_cflags) \
        -include camera_defs_i.h


LOCAL_C_INCLUDES += chromatix_ov13850_snapshot.h


LOCAL_SRC_FILES:= chromatix_ov13850_snapshot.c


LOCAL_MODULE           := libchromatix_ov13850_snapshot_experimental


LOCAL_SHARED_LIBRARIES := libcutils


ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES += liblog
endif


LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true


include $(BUILD_SHARED_LIBRARY)
