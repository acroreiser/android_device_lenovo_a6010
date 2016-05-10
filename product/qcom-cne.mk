# CNE
PRODUCT_PACKAGES += \
    libcnefeatureconfig

ifeq ($(BOARD_USES_QCNE),true)
PRODUCT_PROPERTY_OVERRIDES += \
    persist.cne.feature=4
endif
