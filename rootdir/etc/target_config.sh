#!/system/bin/sh

ZMEM=$(cat /proc/meminfo | grep MemTotal | awk '{print $2}')

# Tune the system according to RAM size
if [ "$ZMEM" -gt "1100000" ]; then
	setprop persist.device_config.runtime_native.usap_pool_enabled true
	setprop ro.vendor.qti.sys.fw.trim_enable_memory 2147483648
	setprop ro.vendor.qti.sys.fw.bg_apps_limit 16
	setprop dalvik.vm.usejit true
	setprop dalvik.vm.usejitprofiles true
	setprop pm.dexopt.install speed-profile
else
	setprop persist.device_config.runtime_native.usap_pool_enabled false
	setprop ro.vendor.qti.sys.fw.trim_enable_memory 1073741824
	setprop ro.vendor.qti.sys.fw.bg_apps_limit 4
	setprop dalvik.vm.usejit false
	setprop dalvik.vm.usejitprofiles false
	setprop pm.dexopt.install speed
	setprop pm.dexopt.bg-dexopt speed
fi