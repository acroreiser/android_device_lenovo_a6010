TARGET_LD_SHIM_LIBS := \
    /system/vendor/lib/libflp.so|libshims_flp.so \
    /system/vendor/lib/libizat_core.so|libshims_get_process_name.so \
    /system/bin/mm-qcamera-daemon|libshim_atomic.so \
    /system/vendor/lib/libmmcamera2_imglib_modules.so|libshim_atomic.so \
    /system/vendor/lib/libqomx_jpegenc.so|libshim_atomic.so \
    /system/vendor/lib/libmmcamera2_stats_modules.so|libshim_atomic.so \
    /system/lib/libfacenet.so|libprotobuf-cpp-haxx.so
