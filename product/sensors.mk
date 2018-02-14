# Sensors
PRODUCT_PACKAGES += \
    android.hardware.sensors@1.0-impl \
    accelcal \
    AccCalibration \
    sensord \
    sensors.msm8916

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:system/vendor/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:system/vendor/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:system/vendor/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:system/vendor/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:system/vendor/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/native/data/etc/handheld_core_hardware.xml:system/vendor/etc/permissions/handheld_core_hardware.xml \
    $(LOCAL_PATH)/configs/sensors/_hals.conf:system/vendor/etc/sensors/_hals.conf
