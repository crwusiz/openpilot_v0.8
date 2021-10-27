#!/usr/bin/bash

if [ -z "$BASEDIR" ]; then
  BASEDIR="/data/openpilot"
fi

source "$BASEDIR/launch_env.sh"
cp -f "$BASEDIR/installer/updater/update.zip" "/sdcard/update.zip"
pm disable ai.comma.plus.offroad
killall _ui
"$BASEDIR/selfdrive/hardware/eon/updater" "file://$BASEDIR/selfdrive/hardware/eon/op3t.json"
