# Init scripts
PRODUCT_PACKAGES += \
    init.qcom.rc \
    init.qcom.power.rc \
    init.recovery.qcom.rc \
    ueventd.qcom.rc

ifeq ($(TARGET_BUILD_VARIANT),user)
PRODUCT_PACKAGES += \
    init.qcom.ssr.rc
endif
