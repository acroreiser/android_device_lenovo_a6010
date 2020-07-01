#! /vendor/bin/sh

VER="$(cat /proc/version | grep lineage)"
echo $VER
if [ -z "$VER" ]; then
    VER="$(cat /proc/version | grep Eclipse)"
    if [ -z "$VER" ]; then
        /system/bin/reboot
    fi
fi

# Setup ZRAM
swapoff /dev/block/zram0
sleep 0.5
ZMEM=$(cat /proc/meminfo | grep MemTotal | awk  '{print $2}')
let 'ZMEM=((ZMEM/100)*50)*1024'
echo 1 > /sys/block/zram0/reset
echo 'lz4' > /sys/block/zram0/comp_algorithm
sleep 0.5
echo $ZMEM > /sys/block/zram0/disksize
sleep 0.5
mkswap /dev/block/zram0
sleep 0.5
swapon /dev/block/zram0
echo 100 > /proc/sys/vm/swappiness

