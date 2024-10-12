#!/system/bin/sh

setprop ro.vendor.qti.sys.fw.trim_enable_memory 2147483648
setprop persist.device_config.runtime_native.usap_pool_enabled false
setprop ro.vendor.qti.sys.fw.bg_apps_limit 16
setprop pm.dexopt.bg-dexopt speed-profile
setprop pm.dexopt.install speed-profile
setprop dalvik.vm.usejit true
setprop dalvik.vm.usejitprofiles true
setprop dalvik.vm.heapgrowthlimit 80m
setprop dalvik.vm.heapstartsize 4m

echo 50000 > /proc/sys/kernel/sched_freq_inc_notify
echo 50000 > /proc/sys/kernel/sched_freq_dec_notify
echo 100 > /proc/sys/kernel/sched_upmigrate
echo 0 > /proc/sys/kernel/sched_downmigrate
echo 0 > /proc/sys/kernel/sched_window_stats_policy
echo 2 > /proc/sys/kernel/sched_ravg_hist_size

echo 0 > /proc/sys/kernel/sched_enable_power_aware

echo 0 > /proc/sys/kernel/sched_small_task
echo 50 > /proc/sys/kernel/sched_init_task_load
echo 15 > /proc/sys/kernel/sched_spill_nr_run
echo 30 > /proc/sys/kernel/sched_mostly_idle_load

setprop persist.audio.calfile0 /vendor/etc/acdbdata/QRD/QRD_Bluetooth_cal.acdb
setprop persist.audio.calfile1 /vendor/etc/acdbdata/QRD/QRD_General_cal.acdb
setprop persist.audio.calfile2 /vendor/etc/acdbdata/QRD/QRD_Global_cal.acdb
setprop persist.audio.calfile3 /vendor/etc/acdbdata/QRD/QRD_Handset_cal.acdb
setprop persist.audio.calfile4 /vendor/etc/acdbdata/QRD/QRD_Hdmi_cal.acdb
setprop persist.audio.calfile5 /vendor/etc/acdbdata/QRD/QRD_Headset_cal.acdb
setprop persist.audio.calfile6 /vendor/etc/acdbdata/QRD/QRD_Speaker_cal.acdb

if [ "$(getprop ro.camera.experimental_libs)" == "true" ]; then
  mount -o bind /vendor/lib/libchromatix_ov13850_snapshot_experimental.so /vendor/lib/libchromatix_ov13850_snapshot.so
  mount -o bind /vendor/lib/libmmcamera_ov13850_experimental.so /vendor/lib/libmmcamera_ov13850.so
fi
