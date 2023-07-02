#! /vendor/bin/sh

# move cpu-hungry kswapd and mmcqd/0 tasks to background cpuset
echo $(pgrep kswapd) > /dev/cpuset/background/tasks
echo $(pgrep mmcqd/0 | head -n 1) > /dev/cpuset/background/tasks

# move UI-related threads to top-app cpuset
echo $(pgrep mdss_dsi_event) > /dev/cpuset/top-app/tasks
echo $(pgrep kgsl_worker_thr) > /dev/cpuset/top-app/tasks

# move watchdog thread to foregrond cpuset
echo $(pgrep msm_watchdog) > /dev/cpuset/foreground/tasks

