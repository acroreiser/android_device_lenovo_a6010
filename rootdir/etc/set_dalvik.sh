#! /vendor/bin/sh

# Copyright (c) 2019 Harshit Jain
#
# Dalvik Prop Script
#

PATH=/sbin:/vendor/sbin:/vendor/bin:/vendor/xbin
export PATH

# Get Memtotal
ram=`cat /proc/meminfo | grep MemTotal`

# Initially Set this to 1024
IsLow=1024

# Change to 2048 for 2G
if [[ $ram > 1048576 ]]; then
    IsLow=2048;
fi

dalvik_setup ()
{
    case $IsLow in
        # 1GB
        1024 )
            setprop dalvik.vm.heapstartsize "16m"
            setprop dalvik.vm.heapgrowthlimit "128m"
            setprop dalvik.vm.heapsize "256m"
            setprop dalvik.vm.heaptargetutilization "0.75"
            setprop dalvik.vm.heapminfree "512k"
            setprop dalvik.vm.heapmaxfree "8m"
        ;;
        # 2GB
        * )
            setprop dalvik.vm.heapstartsize "16m"
            setprop dalvik.vm.heapgrowthlimit "256m"
            setprop dalvik.vm.heapsize "512m"
            setprop dalvik.vm.heaptargetutilization "0.75"
            setprop dalvik.vm.heapminfree "2m"
            setprop dalvik.vm.heapmaxfree "8m"
        ;;
    esac
}

dalvik_setup

return 0