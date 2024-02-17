#!/system/vendor/bin/sh
#
# Script to setup many things after boot

# Setup CPUFreq
echo "schedutil" > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
echo 500 > /sys/devices/system/cpu/cpufreq/schedutil/up_rate_limit_us
echo 20000 > /sys/devices/system/cpu/cpufreq/schedutil/down_rate_limit_us
echo 85 > /sys/devices/system/cpu/cpufreq/schedutil/hispeed_load

# Setup CPU-boost
echo 0:1209600 > /sys/module/cpu_boost/parameters/input_boost_freq
echo 0:1209600 > /sys/module/cpu_boost/parameters/powerkey_input_boost_freq

/vendor/bin/timekeep restore

# Harden memfd_create
sysctl -w vm.memfd_noexec=1

# Tune task scheduler to prefer idle cpus to place task
sysctl -w kernel.sched_prefer_idle=1

# Tune task scheduler to reduce possibility to pick some heavy task
# to the same cpu with ui-threads
sysctl -w kernel.sched_spill_load=85

echo 1 > /sys/devices/platform/kcal_ctrl.0/kcal_enable
echo 261 > /sys/devices/platform/kcal_ctrl.0/kcal_sat
