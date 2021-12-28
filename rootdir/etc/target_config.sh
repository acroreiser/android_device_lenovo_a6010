#!/system/bin/sh

ZMEM=$(cat /proc/meminfo | grep MemTotal | awk '{print $2}')

# Tune the system according to RAM size
if [ "$ZMEM" -gt "1100000" ]; then
	setprop persist.device_config.runtime_native.usap_pool_enabled true
	setprop ro.vendor.qti.sys.fw.trim_enable_memory 2147483648
	setprop ro.vendor.qti.sys.fw.bg_apps_limit 16
else
	setprop persist.device_config.runtime_native.usap_pool_enabled false
	setprop ro.vendor.qti.sys.fw.trim_enable_memory 1073741824
	setprop ro.vendor.qti.sys.fw.bg_apps_limit 8
fi

	setprop pm.dexopt.bg-dexopt speed-profile
	setprop pm.dexopt.install speed-profile
	setprop dalvik.vm.usejit true
	setprop dalvik.vm.usejitprofiles true

# Don't account allocstalls for <= 2GB RAM targets on kernel versions < 4.9
echo 100 > /sys/module/vmpressure/parameters/allocstall_threshold
