# Healthd
PRODUCT_PACKAGES += \
    android.hardware.health@1.0-convert \
    android.hardware.health@1.0-impl \
    android.hardware.health@1.0-service

# Charger
PRODUCT_COPY_FILES += \
    device/lenovo/msm8916-common/rootdir/charger/chargeonlymode:$(TARGET_OUT_DIR)/root/sbin/charger \
    device/lenovo/msm8916-common/rootdir/charger/battery_image_0.png:$(TARGET_OUT_DIR)/root/res/images/charger/battery_image_0.png \
    device/lenovo/msm8916-common/rootdir/charger/battery_image_1.png:$(TARGET_OUT_DIR)/root/res/images/charger/battery_image_1.png \
    device/lenovo/msm8916-common/rootdir/charger/battery_image_2.png:$(TARGET_OUT_DIR)/root/res/images/charger/battery_image_2.png \
    device/lenovo/msm8916-common/rootdir/charger/battery_image_3.png:$(TARGET_OUT_DIR)/root/res/images/charger/battery_image_3.png \
    device/lenovo/msm8916-common/rootdir/charger/battery_image_4.png:$(TARGET_OUT_DIR)/root/res/images/charger/battery_image_4.png \
    device/lenovo/msm8916-common/rootdir/charger/battery_image_5.png:$(TARGET_OUT_DIR)/root/res/images/charger/battery_image_5.png \
    device/lenovo/msm8916-common/rootdir/charger/battery_image_6.png:$(TARGET_OUT_DIR)/root/res/images/charger/battery_image_6.png \
    device/lenovo/msm8916-common/rootdir/charger/battery_image_7.png:$(TARGET_OUT_DIR)/root/res/images/charger/battery_image_7.png \
    device/lenovo/msm8916-common/rootdir/charger/battery_num_0.png:$(TARGET_OUT_DIR)/root/res/images/charger/battery_num_0.png \
    device/lenovo/msm8916-common/rootdir/charger/battery_num_1.png:$(TARGET_OUT_DIR)/root/res/images/charger/battery_num_1.png \
    device/lenovo/msm8916-common/rootdir/charger/battery_num_2.png:$(TARGET_OUT_DIR)/root/res/images/charger/battery_num_2.png \
    device/lenovo/msm8916-common/rootdir/charger/battery_num_3.png:$(TARGET_OUT_DIR)/root/res/images/charger/battery_num_3.png \
    device/lenovo/msm8916-common/rootdir/charger/battery_num_4.png:$(TARGET_OUT_DIR)/root/res/images/charger/battery_num_4.png \
    device/lenovo/msm8916-common/rootdir/charger/battery_num_5.png:$(TARGET_OUT_DIR)/root/res/images/charger/battery_num_5.png \
    device/lenovo/msm8916-common/rootdir/charger/battery_num_6.png:$(TARGET_OUT_DIR)/root/res/images/charger/battery_num_6.png \
    device/lenovo/msm8916-common/rootdir/charger/battery_num_7.png:$(TARGET_OUT_DIR)/root/res/images/charger/battery_num_7.png \
    device/lenovo/msm8916-common/rootdir/charger/battery_num_8.png:$(TARGET_OUT_DIR)/root/res/images/charger/battery_num_8.png \
    device/lenovo/msm8916-common/rootdir/charger/battery_num_9.png:$(TARGET_OUT_DIR)/root/res/images/charger/battery_num_9.png \
    device/lenovo/msm8916-common/rootdir/charger/battery_num_10.png:$(TARGET_OUT_DIR)/root/res/images/charger/battery_num_10.png
