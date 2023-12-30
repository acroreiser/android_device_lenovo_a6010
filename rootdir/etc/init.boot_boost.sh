#!/system/vendor/bin/sh
#
# Script to setup cpus (sd-410) after boot

echo "schedutil" > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
echo 5000 > /sys/devices/system/cpu/cpufreq/schedutil/down_rate_limit_us
echo 90 > /sys/devices/system/cpu/cpufreq/schedutil/hispeed_load
