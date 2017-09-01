# Boot animation
TARGET_SCREEN_HEIGHT := 1280
TARGET_SCREEN_WIDTH := 720

# Panel
PRODUCT_PACKAGES += \
    libjni_livedisplay

# Screen density
PRODUCT_PROPERTY_OVERRIDES += \
    ro.sf.lcd_density=320

PRODUCT_AAPT_CONFIG := normal
PRODUCT_AAPT_PREF_CONFIG := xhdpi

#HWUI Props
PRODUCT_PROPERTY_OVERRIDES += \
    ro.hwui.texture_cache_size=24 \
    ro.hwui.layer_cache_size=16 \
    ro.hwui.r_buffer_cache_size=2 \
    ro.hwui.path_cache_size=4 \
    ro.hwui.gradient_cache_size=0.5 \
    ro.hwui.drop_shadow_cache_size=2 \
    ro.hwui.texture_cache_flushrate=0.6 \
    ro.hwui.text_small_cache_width=1024 \
    ro.hwui.text_small_cache_height=256 \
    ro.hwui.text_large_cache_width=2048 \
    ro.hwui.text_large_cache_height=512