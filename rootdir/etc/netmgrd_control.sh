#!/system/bin/sh
# Re-trigger netmgrd when libc crashes
if [ `getprop sys.boot_completed` -eq 1 ]; then
    for pid in `pidof /system/bin/rild`
    do
        kill $pid
    done
fi
/system/bin/netmgrd
