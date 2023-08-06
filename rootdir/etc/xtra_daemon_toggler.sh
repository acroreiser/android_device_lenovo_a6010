#! /vendor/bin/sh

if [ "$(getprop persist.sys.xtra-daemon.enabled)" == "true" ]; then
    if [ "$(getprop sys.xtra.mounted)" != "true" ]; then
        mount --bind /vendor/etc/izat_xtra.conf /vendor/etc/izat.conf
        setprop sys.xtra.mounted true
    fi
else
    if [ "$(getprop sys.xtra.mounted)" == "true" ]; then
        umount /vendor/etc/izat.conf
        setprop sys.xtra.mounted false
    fi
fi
