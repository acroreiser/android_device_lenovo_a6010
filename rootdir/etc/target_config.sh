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

# Don't account allocstalls for <= 2GB RAM targets on kernel versions < 4.9
echo 100 > /sys/module/vmpressure/parameters/allocstall_threshold

echo 50000 > /proc/sys/kernel/sched_freq_inc_notify
echo 50000 > /proc/sys/kernel/sched_freq_dec_notify
echo 75 > /proc/sys/kernel/sched_upmigrate
echo 60 > /proc/sys/kernel/sched_downmigrate
echo 3 > /proc/sys/kernel/sched_window_stats_policy
echo 3 > /proc/sys/kernel/sched_ravg_hist_size

echo 20 > /proc/sys/kernel/sched_small_task
echo 15 > /proc/sys/kernel/sched_spill_nr_run
echo 30 > /proc/sys/kernel/sched_mostly_idle_load

echo 10 > /sys/module/process_reclaim/parameters/pressure_min
echo 70 > /sys/module/process_reclaim/parameters/pressure_max

setprop persist.audio.calfile0 /vendor/etc/acdbdata/QRD/QRD_Bluetooth_cal.acdb
setprop persist.audio.calfile1 /vendor/etc/acdbdata/QRD/QRD_General_cal.acdb
setprop persist.audio.calfile2 /vendor/etc/acdbdata/QRD/QRD_Global_cal.acdb
setprop persist.audio.calfile3 /vendor/etc/acdbdata/QRD/QRD_Handset_cal.acdb
setprop persist.audio.calfile4 /vendor/etc/acdbdata/QRD/QRD_Hdmi_cal.acdb
setprop persist.audio.calfile5 /vendor/etc/acdbdata/QRD/QRD_Headset_cal.acdb
setprop persist.audio.calfile6 /vendor/etc/acdbdata/QRD/QRD_Speaker_cal.acdb

/vendor/bin/timekeep restore

sysctl -w vm.memfd_noexec=1

# Tune task scheduler to prefer idle cpus to place task
sysctl -w kernel.sched_prefer_idle=1

# Tune task scheduler to reduce possibility to pick some heavy task
# to the same cpu with ui-threads
sysctl -w kernel.sched_spill_load=85
