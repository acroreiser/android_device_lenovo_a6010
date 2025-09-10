#!/system/vendor/bin/sh
#
# Script to setup many things after boot

# Setup CPUFreq
echo "schedutil" > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
echo 0 > /sys/devices/system/cpu/cpufreq/schedutil/up_rate_limit_us
echo 300 > /sys/devices/system/cpu/cpufreq/schedutil/down_rate_limit_us
echo 99 > /sys/devices/system/cpu/cpufreq/schedutil/hispeed_load
echo 1209600 > /sys/devices/system/cpu/cpufreq/schedutil/hispeed_freq

# Setup CPU-boost
echo 0:0 > /sys/module/cpu_boost/parameters/input_boost_freq
echo 0 > /sys/module/cpu_boost/parameters/input_boost_ms
echo 0:1209600 > /sys/module/cpu_boost/parameters/powerkey_input_boost_freq

/vendor/bin/timekeep restore

# Harden memfd_create
sysctl -w vm.memfd_noexec=1

echo 5 > /dev/cpuctl/background/cpu.uclamp.max
echo 40 > /dev/cpuctl/system-background/cpu.uclamp.max
echo 50 > /dev/cpuctl/foreground/cpu.uclamp.max
echo 30 > /dev/cpuctl/foreground/cpu.uclamp.min
echo 5 > /dev/cpuctl/dex2oat/cpu.uclamp.max
echo max > /dev/cpuctl/top-app/cpu.uclamp.min
echo 1 > /dev/cpuctl/top-app/cpu.uclamp.latency_sensitive
echo max > /dev/cpuctl/camera-daemon/cpu.uclamp.min
echo 1 > /dev/cpuctl/camera-daemon/cpu.uclamp.latency_sensitive

# Tune task scheduler to reduce possibility to pick some heavy task
# to the same cpu with ui-threads
sysctl -w kernel.sched_spill_load=100

sysctl -w kernel.sched_rr_timeslice_ms=50

sysctl -w kernel.sched_min_granularity_ns=3000000
sysctl -w kernel.sched_latency_ns=100000

echo 1 > /sys/devices/platform/kcal_ctrl.0/kcal_enable
echo 261 > /sys/devices/platform/kcal_ctrl.0/kcal_sat

# Virtual memory management
echo 0 > /proc/sys/vm/extra_free_kbytes
echo 230 > /proc/sys/vm/watermark_scale_factor
echo 10572 > /proc/sys/vm/min_free_kbytes
echo 0 > /proc/sys/vm/admin_reserve_kbytes
echo 0 > /proc/sys/vm/user_reserve_kbytes

echo 52428800 > /proc/sys/vm/dirty_bytes
echo 4194304 > /proc/sys/vm/dirty_background_bytes

# Set allocstall_threshold to 0
echo 0 > /sys/module/vmpressure/parameters/allocstall_threshold

# Set kswapd threads
echo 1 > /proc/sys/vm/kswapd_threads

# Set KGSL pwrlevel
echo 2 > /sys/class/kgsl/kgsl-3d0/default_pwrlevel

# Limit cputime for background tasks
echo 1000000 > /dev/cpuctl/background/cpu.cfs_period_us
echo 2000000 > /dev/cpuctl/background/cpu.cfs_quota_us

# Limit cputime for dex2oat
echo 1000000 > /dev/cpuctl/dex2oat/cpu.cfs_period_us
echo 2000000 > /dev/cpuctl/dex2oat/cpu.cfs_quota_us

# Setup cpu.shares to throttle background groups (bg ~ 5% sysbg ~ 5% dex2oat ~2.5%)
echo 1024 > /dev/cpuctl/background/cpu.shares
echo 1024 > /dev/cpuctl/system-background/cpu.shares
echo 512 > /dev/cpuctl/dex2oat/cpu.shares
echo 20480 > /dev/cpuctl/foreground/cpu.shares
echo 20480 > /dev/cpuctl/top-app/cpu.shares
echo 20480 > /dev/cpuctl/camera-daemon/cpu.shares
echo 20480 > /dev/cpuctl/system/cpu.shares
echo 20480 > /dev/cpuctl/nnapi-hal/cpu.shares
echo 20480 > /dev/cpuctl/rt/cpu.shares

echo 0 > /sys/block/mmcblk0/queue/iosched/back_seek_max
echo 0 > /sys/block/mmcblk0/queue/iosched/back_seek_penalty
echo 128 > /sys/block/mmcblk0/queue/nr_requests
echo 128 > /sys/block/mmcblk0/queue/read_ahead_kb
echo 1 > /sys/block/mmcblk0/queue/iosched/strict_guarantees
echo 150000 > /sys/block/mmcblk0/queue/iosched/max_budget

echo 0 > /sys/block/mmcblk1/queue/iosched/back_seek_max
echo 0 > /sys/block/mmcblk1/queue/iosched/back_seek_penalty
echo 128 > /sys/block/mmcblk1/queue/nr_requests
echo 128 > /sys/block/mmcblk1/queue/read_ahead_kb
echo 1 > /sys/block/mmcblk1/queue/iosched/strict_guarantees
