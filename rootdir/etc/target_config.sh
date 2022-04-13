#!/system/bin/sh

ZMEM=$(cat /proc/meminfo | grep MemTotal | awk '{print $2}')

# Tune the system according to RAM size
if [ "$ZMEM" -gt "1100000" ]; then
	setprop persist.device_config.runtime_native.usap_pool_enabled true
	setprop ro.vendor.qti.sys.fw.trim_enable_memory 2147483648
else
	setprop persist.device_config.runtime_native.usap_pool_enabled false
	setprop ro.vendor.qti.sys.fw.trim_enable_memory 1073741824
fi

	setprop ro.vendor.qti.sys.fw.bg_apps_limit 8

	setprop pm.dexopt.bg-dexopt speed-profile
	setprop pm.dexopt.install speed-profile
	setprop dalvik.vm.usejit true
	setprop dalvik.vm.usejitprofiles true
	setprop dalvik.vm.heapgrowthlimit 80m
	setprop dalvik.vm.heapstartsize 4m

    setprop persist.audio.calfile0 /vendor/etc/acdbdata/QRD/QRD_Bluetooth_cal.acdb
    setprop persist.audio.calfile1 /vendor/etc/acdbdata/QRD/QRD_General_cal.acdb
    setprop persist.audio.calfile2 /vendor/etc/acdbdata/QRD/QRD_Global_cal.acdb
    setprop persist.audio.calfile3 /vendor/etc/acdbdata/QRD/QRD_Handset_cal.acdb
    setprop persist.audio.calfile4 /vendor/etc/acdbdata/QRD/QRD_Hdmi_cal.acdb
    setprop persist.audio.calfile5 /vendor/etc/acdbdata/QRD/QRD_Headset_cal.acdb
    setprop persist.audio.calfile6 /vendor/etc/acdbdata/QRD/QRD_Speaker_cal.acdb

# Don't account allocstalls for <= 2GB RAM targets on kernel versions < 4.9
echo 100 > /sys/module/vmpressure/parameters/allocstall_threshold
