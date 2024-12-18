#!/system/vendor/bin/sh
#
# Script to setup many things after boot

# Setup CPUFreq
echo "schedutil" > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
echo 0 > /sys/devices/system/cpu/cpufreq/schedutil/up_rate_limit_us
echo 5000 > /sys/devices/system/cpu/cpufreq/schedutil/down_rate_limit_us
echo 70 > /sys/devices/system/cpu/cpufreq/schedutil/hispeed_load

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
sysctl -w kernel.sched_spill_load=100

echo 1 > /sys/devices/platform/kcal_ctrl.0/kcal_enable
echo 261 > /sys/devices/platform/kcal_ctrl.0/kcal_sat

# Virtual memory management
echo 0 > /proc/sys/vm/extra_free_kbytes
echo 230 > /proc/sys/vm/watermark_scale_factor
echo 10572 > /proc/sys/vm/min_free_kbytes

# Set allocstall_threshold to 0
echo 0 > /sys/module/vmpressure/parameters/allocstall_threshold

# Set kswapd threads
echo 1 > /proc/sys/vm/kswapd_threads

# Set lowest possible timeslice for SCHED_RR tasks
echo 1 > /proc/sys/kernel/sched_rr_timeslice_ms

# Set KGSL pwrlevel
echo 2 > /sys/class/kgsl/kgsl-3d0/default_pwrlevel

# Limit background tasks IO bandwidth
echo "179:0 8388608" > /dev/blkio/background/blkio.throttle.read_bps_device
echo "179:0 4194304" > /dev/blkio/background/blkio.throttle.write_bps_device
echo "179:64 8388608" > /dev/blkio/background/blkio.throttle.read_bps_device
echo "179:64 4194304" > /dev/blkio/background/blkio.throttle.write_bps_device

# Limit cputime for background tasks
echo 1000000 > /dev/cpuctl/background/cpu.cfs_period_us
echo 2000000 > /dev/cpuctl/background/cpu.cfs_quota_us

# Limit cputime for foreground tasks
echo 1000000 > /dev/cpuctl/background/cpu.cfs_period_us
echo 3000000 > /dev/cpuctl/background/cpu.cfs_quota_us
