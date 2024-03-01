#! /vendor/bin/sh

# move janky tasks to background cpuset
echo $(pgrep mmcqd/0 | head -n 1) > /dev/cpuset/background/tasks
echo $(pgrep msm_watchdog) > /dev/cpuset/background/tasks
echo $(pgrep irqbalance) > /dev/cpuset/background/tasks
echo $(pgrep rild | head -n 1) > /dev/cpuset/background/tasks
echo $(pgrep rild | head -n 2 | sed /$(head -n1)/d) > /dev/cpuset/background/tasks
echo $(pgrep qmuxd) > /dev/cpuset/background/tasks
echo $(pgrep netmgrd) > /dev/cpuset/background/tasks

echo $(pgrep logd) > /dev/cpuset/background/tasks
chrt -p $(pgrep logd) -i 0

echo $(pgrep wcnss_service) > /dev/cpuset/background/tasks

# move critical and UI-related tasks to top-app cpuset
echo $(pgrep composer) > /dev/cpuset/top-app/tasks
echo $(pgrep mdss_dsi_event) > /dev/cpuset/top-app/tasks
echo $(pgrep kgsl_worker_thr) > /dev/cpuset/top-app/tasks
