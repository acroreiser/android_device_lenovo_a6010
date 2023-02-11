#! /vendor/bin/sh

TMEM=$(cat /proc/meminfo | grep MemTotal | awk  '{print $2}')

# Reduce io thrashing
if [ "$TMEM" -gt "1100000" ]; then
	echo "50" > /proc/sys/vm/vmpressure_level_med
fi

# Reserve 10% of memory for clean pages
let 'CMEM=(TMEM/100)*10'
echo $CMEM > /proc/sys/vm/clean_min_kbytes

# Setup ZRAM to 75% of memtotal
let 'ZMEM=((TMEM/100)*75)*1024'
echo 1 > /sys/block/zram0/reset
echo 'zstd' > /sys/block/zram0/comp_algorithm
sleep 0.5
echo $ZMEM > /sys/block/zram0/disksize
sleep 0.5
mkswap /dev/block/zram0
sleep 0.5
swapon -d /dev/block/zram0
echo 150 > /proc/sys/vm/swappiness
