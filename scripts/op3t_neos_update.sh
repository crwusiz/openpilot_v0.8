#!/usr/bin/bash

if [ -z "$BASEDIR" ]; then
  BASEDIR="/data/openpilot"
fi
source "$BASEDIR/launch_env.sh"

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
NEOS_PY="$DIR/selfdrive/hardware/eon/neos.py"
MANIFEST="$DIR/selfdrive/hardware/eon/op3t.json"

echo "Installing op3t NEOS update"
cp -f "$BASEDIR/installer/updater/update.zip" "/sdcard/update.zip"
pm disable ai.comma.plus.offroad
killall _ui

#"$BASEDIR/selfdrive/hardware/eon/updater" "file://$BASEDIR/selfdrive/hardware/eon/op3t.json"
$NEOS_PY --swap-if-ready $MANIFEST
$DIR/selfdrive/hardware/eon/updater $NEOS_PY $MANIFEST
