ifeq ($(call my-dir),$(call project-path-for,qcom-display))
display-hals := include libgralloc libgenlock libcopybit
display-hals += libhwcomposer liboverlay libqdutils libhdmi libqservice
display-hals += libmemtrack
ifneq ($(TARGET_PROVIDES_LIBLIGHT),true)
display-hals += liblight
endif
ifeq ($(call is-vendor-board-platform,QCOM),true)
    include $(call all-named-subdir-makefiles,$(display-hals))
else
ifneq ($(filter msm% apq%,$(TARGET_BOARD_PLATFORM)),)
    include $(call all-named-subdir-makefiles,$(display-hals))
endif
endif

include $(CLEAR_VARS)
LOCAL_MODULE := display_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := \
    $(display_top)/include \
    $(display_top)/libcopybit \
    $(display_top)/libhdmi \
    $(display_top)/libhwcomposer \
    $(display_top)/liboverlay \
    $(display_top)/libqdutils \
    $(display_top)/libqservice

ifeq ($(TARGET_USES_GRALLOC1), true)
    LOCAL_EXPORT_C_INCLUDE_DIRS += $(display_top)/libgralloc1
else
    LOCAL_EXPORT_C_INCLUDE_DIRS += $(display_top)/libgralloc
endif

include $(BUILD_HEADER_LIBRARY)

endif
