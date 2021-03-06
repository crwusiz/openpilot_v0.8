#!/usr/bin/bash

export LD_LIBRARY_PATH=/data/data/com.termux/files/usr/lib
export HOME=/data/data/com.termux/files/home
export PATH=/usr/local/bin:/data/data/com.termux/files/usr/bin:/data/data/com.termux/files/usr/sbin:/data/data/com.termux/files/usr/bin/applets:/bin:/sbin:/vendor/bin:/system/sbin:/system/bin:/system/xbin:/data/data/com.termux/files/usr/bin/python
export PYTHONPATH=/data/openpilot

cd /data/openpilot && /data/data/com.termux/files/usr/bin/git fetch --all; /data/data/com.termux/files/usr/bin/git reset --hard HEAD; /data/data/com.termux/files/usr/bin/git pull;
/data/data/com.termux/files/usr/bin/git log -1; 
cd /sdcard && rm -r realdata;
cd /data/openpilot/installer/fonts && cp driver_monitor.py /data/openpilot/selfdrive/monitoring; reboot
