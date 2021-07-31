#! /vendor/bin/sh

# grep the modem partition for baseband version and set and cache it
# if modem_version.txt is present, cat it and use

# use QC_IMAGE_VERSION_STRING as unified for all 8916 modem images
# grep all modem files for first time

if [ -f "/data/modem_version.txt" ]; then
    setprop gsm.version.baseband `cat /data/modem_version.txt`
else
    echo `strings /firmware/image/modem.b* | grep "QC_IMAGE_VERSION_STRING" | head -1  | sed s/QC_IMAGE_VERSION_STRING=//` > /data/modem_version.txt
    setprop gsm.version.baseband `cat /data/modem_version.txt`
fi
