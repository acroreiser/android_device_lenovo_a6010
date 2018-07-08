# Optimize
PRODUCT_SYSTEM_SERVER_COMPILER_FILTER := speed-profile
PRODUCT_ALWAYS_PREOPT_EXTRACTED_APK := true
PRODUCT_USE_PROFILE_FOR_BOOT_IMAGE := true
PRODUCT_DEX_PREOPT_BOOT_IMAGE_PROFILE_LOCATION := frameworks/base/config/boot-image-profile.txt

PRODUCT_PROPERTY_OVERRIDES += \
    dalvik.vm.heapgrowthlimit=128m \
    dalvik.vm.heapminfree=6m \
    dalvik.vm.heapstartsize=8m \
    dalvik.vm.heapsize=256m \
    dalvik.vm.heaptargetutilization=0.75 \
    dalvik.vm.heapmaxfree=8m \
    dalvik.vm.zygotemaxfailedboots=5 \
    dalvik.vm.foreground-heap-growth-multiplier=2.0 \
    dalvik.vm.dex2oat-flags=--no-watch-dog \
    dalvik.vm.dex2oat-swap=false \
    dalvik.vm.dex2oat-threads=2 \
    ro.vendor.qti.am.reschedule_service=true \
    sys.use_fifo_ui=1

# TextClassifier
PRODUCT_PACKAGES += \
    textclassifier.smartselection.bundle1
