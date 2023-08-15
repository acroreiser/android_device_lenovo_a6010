#! /vendor/bin/sh

TMEM=$(cat /proc/meminfo | grep MemTotal | awk  '{print $2}')

# Reduce io thrashing
echo "50" > /proc/sys/vm/vmpressure_level_med

# Reserve 10% of memory for clean pages
let 'CMEM=(TMEM/100)*10'
echo $CMEM > /proc/sys/vm/clean_min_kbytes

# Setup ZRAM to 75% of memtotal
let 'ZMEM=((TMEM/100)*75)*1024'
echo 1 > /sys/block/zram0/reset
echo 'lz4' > /sys/block/zram0/comp_algorithm
sleep 0.5
echo $ZMEM > /sys/block/zram0/disksize
sleep 0.5
mkswap /dev/block/zram0
sleep 0.5
swapon -d /dev/block/zram0
echo 150 > /proc/sys/vm/swappiness

# Configure lowmemorykiller
echo 0,50,100,200,250,300,400,500,600,700,800,900,950 > /sys/module/lowmemorykiller/parameters/adj
echo 209715,230686,251658,272629,293601,314572,335544,356515,377487,398458,419430,440401,1048576 > /sys/module/lowmemorykiller/parameters/minfree
