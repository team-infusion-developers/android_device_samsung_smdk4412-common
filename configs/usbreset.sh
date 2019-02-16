#!/system/bin/sh

write()
{
    echo $2 > $1
}

reinit()
{
    if [ $(cat /sys/class/android_usb/android0/state) != "CONFIGURED" ] ; then
       write /sys/class/android_usb/android0/enable 0
       write /sys/class/android_usb/android0/enable 1
    fi
}

reinit
reinit
