#! /vendor/bin/sh

TMEM=$(cat /proc/meminfo | grep MemTotal | awk  '{print $2}')

# Setup ZRAM to 75% of memtotal
let 'ZMEM=((TMEM/100)*75)*1024'
echo 1 > /sys/block/zram0/reset
echo 'lz4kd' > /sys/block/zram0/comp_algorithm
echo $ZMEM > /sys/block/zram0/disksize
mkswap /dev/block/zram0
swapon -d /dev/block/zram0
echo 150 > /proc/sys/vm/swappiness

# Set kswapd threads
echo 4 > /proc/sys/vm/kswapd_threads
