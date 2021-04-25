#!/system/bin/sh
#
# By acroreiser, 2021
#
# Workaround to fit encryption footer in userdata.
#

USERDATABLK=/dev/block/mmcblk0p31

SIZE=$(tune2fs -l $USERDATABLK | grep 'Block count' | awk  '{ print $3 }')

if [ -z "$SIZE" ]; then
    echo "fde_ext4_resize: FS is not ext4 or already encrypted" >> /dev/kmsg
    exit 0
else
    echo "fde_ext4_resize: FS has $SIZE blocks!" >> /dev/kmsg
fi

SZBYMEM=$(cat /proc/meminfo | grep MemTotal | awk  '{print $2}')

if [ "$SZBYMEM" -gt "1100000" ]; then
    echo "fde_ext4_resize: 16G storage device" >> /dev/kmsg

    BASESIZE=2964987
else
    echo "fde_ext4_resize: 8G storage device" >> /dev/kmsg

    BASESIZE=1253367
fi

let "RESIZEDSZ=$BASESIZE - 4"

if [ "$SIZE" == "$BASESIZE" ]; then
    echo "fde_ext4_resize: $USERDATABLK fs will be resized" >> /dev/kmsg
elif [ "$SIZE" == "$RESIZEDSZ" ]; then
    echo "fde_ext4_resize: resizing of $USERDATABLK fs is not required" >> /dev/kmsg
    exit 0
else
    echo "fde_ext4_resize: unexpected filesystem size, aborting..." >> /dev/kmsg
    exit 0
fi

echo "fde_ext4_resize: resizing to $RESIZEDSZ" >> /dev/kmsg

resize2fs -f $USERDATABLK $RESIZEDSZ

echo "fde_ext4_resize: Done!" >> /dev/kmsg