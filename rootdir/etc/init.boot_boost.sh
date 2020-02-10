#!/system/vendor/bin/sh
#
# Script to boost cpus and gpus (sd-410) for 40 sec after boot, to
# reduce lag and bad ux that lasts for about 1 min after booting

echo "performance" > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
echo "performance" > /sys/class/devfreq/1c00000.qcom,kgsl-3d0/governor
echo 400000000 > /sys/class/devfreq/1c00000.qcom,kgsl-3d0/min_freq

sleep 40

echo "interactive" > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
echo "msm-adreno-tz" > /sys/class/devfreq/1c00000.qcom,kgsl-3d0/governor
echo 19200000 > /sys/class/devfreq/1c00000.qcom,kgsl-3d0/min_freq