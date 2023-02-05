#!/system/vendor/bin/sh
#
# Script to boost cpus and gpus (sd-410) for 40 sec after boot, to
# reduce lag and bad ux that lasts for about 1 min after booting

echo "-1" > /proc/sys/kernel/sched_rt_runtime_us

echo "performance" > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
echo "performance" > /sys/class/devfreq/1c00000.qcom,kgsl-3d0/governor
echo 400000000 > /sys/class/devfreq/1c00000.qcom,kgsl-3d0/min_freq

sleep 40

echo "interactive" > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
echo 1 > /sys/devices/system/cpu/cpufreq/interactive/io_is_busy
echo 1 > /sys/devices/system/cpu/cpufreq/interactive/use_migration_notif
echo 1 > /sys/devices/system/cpu/cpufreq/interactive/use_sched_load
echo 1 > /sys/devices/system/cpu/cpufreq/interactive/fast_ramp_down
echo 1 > /sys/devices/system/cpu/cpufreq/interactive/ignore_hispeed_on_notif

echo "msm-adreno-tz" > /sys/class/devfreq/1c00000.qcom,kgsl-3d0/governor
echo 100000000 > /sys/class/devfreq/1c00000.qcom,kgsl-3d0/min_freq
