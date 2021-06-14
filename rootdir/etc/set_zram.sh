#! /vendor/bin/sh

ZMEM=$(cat /proc/meminfo | grep MemTotal | awk  '{print $2}')

# Reduce io thrashing
if [ "$ZMEM" -gt "1100000" ]; then
	echo "50" > /proc/sys/vm/vmpressure_level_med
	echo 81920 > /proc/sys/vm/min_free_kbytes
	echo 81920 > /proc/sys/vm/extra_free_kbytes
else
	echo 40960 > /proc/sys/vm/min_free_kbytes
	echo 40960 > /proc/sys/vm/extra_free_kbytes
fi

# Setup ZRAM to 75% of memtotal
let 'ZMEM=((ZMEM/100)*75)*1024'
echo 1 > /sys/block/zram0/reset
echo 'lz4' > /sys/block/zram0/comp_algorithm
sleep 0.5
echo $ZMEM > /sys/block/zram0/disksize
sleep 0.5
mkswap /dev/block/zram0
sleep 0.5
swapon /dev/block/zram0
echo 100 > /proc/sys/vm/swappiness
