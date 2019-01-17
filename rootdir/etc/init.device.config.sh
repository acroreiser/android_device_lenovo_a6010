#!/vendor/bin/sh

# Copyright (c) 2019 Sultan Qasim Khan
#
# Device configuration for harpia variants
#
PATH=/sbin:/vendor/sbin:/vendor/bin:/vendor/xbin
export PATH

set_config_props ()
{
    setprop ro.config.max_starting_bg "2"

    setprop pm.dexopt.install "quicken"
    setprop pm.dexopt.shared "quicken"
    setprop pm.dexopt.first-boot "verify"

}

set_dalvik_props ()
{
    ram=$(cat /proc/meminfo | grep MemTotal | awk  '{print $2}')

    setprop dalvik.vm.extra-opts "-Xcompiler-option --inline-max-code-units=0"
    setprop dalvik.vm.usejit "true"
    setprop dalvik.vm.usejitprofiles "true"

    if [ "$ram" -gt "1100000" ]; then
        setprop dalvik.vm.heapstartsize "8m"
        setprop dalvik.vm.heapgrowthlimit "192m"
        setprop dalvik.vm.heapsize "512m"
        setprop dalvik.vm.heaptargetutilization "0.75"
        setprop dalvik.vm.heapminfree "512k"
        setprop dalvik.vm.heapmaxfree "8m"
        setprop ro.vendor.qti.sys.fw.bg_apps_limit "12"
    else
        setprop dalvik.vm.heapstartsize "8m"
        setprop dalvik.vm.heapgrowthlimit "192m"
        setprop dalvik.vm.heapsize "256m"
        setprop dalvik.vm.heaptargetutilization "0.75"
        setprop dalvik.vm.heapminfree "2m"
        setprop dalvik.vm.heapmaxfree "8m"
        setprop ro.vendor.qti.sys.fw.bg_apps_limit "6"
        setprop pm.dexopt.downgrade_after_inactive_days "5"
    fi
}

# Main
set_dalvik_props
set_config_props

return 0