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

sysctl -w kernel.sched_window_stats_policy=2
sysctl -w kernel.sched_ravg_hist_size=2

echo 1 > /sys/devices/platform/kcal_ctrl.0/kcal_enable
echo 261 > /sys/devices/platform/kcal_ctrl.0/kcal_sat

# Limit CPU and IO resources for background tasks
echo 102 > /dev/cpuctl/background/cpu.shares
echo 100 > /dev/blkio/background/blkio.weight

# Limit resources for dex2oat
echo 102 > /dev/cpuctl/dex2oat/cpu.shares

# Disable wsf for all targets beacause we are using efk.
# wsf Range : 1..1000 So set to bare minimum value 1.
echo 1 > /proc/sys/vm/watermark_scale_factor
echo 10800 > /proc/sys/vm/extra_free_kbytes

# Set allocstall_threshold to 0
echo 0 > /sys/module/vmpressure/parameters/allocstall_threshold

# Set kswapd threads
echo 4 > /proc/sys/vm/kswapd_threads

echo 5430 > /proc/sys/vm/min_free_kbytes

# Set lowest possible timeslice for SCHED_RR tasks
echo 1 > /proc/sys/kernel/sched_rr_timeslice_ms
