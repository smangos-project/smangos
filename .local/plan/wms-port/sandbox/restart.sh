#!/bin/bash
pkill -f 'godot-mono . -- --project /tmp/wmsdbg' ; sleep 3
rm -f /tmp/wmsdbg/world.doltlite.wms-lock
cd /home/ihm/Documents/Dev/Default/smangos
setsid tools/map_editor/wms-run.sh --no-build --config /tmp/wmsdbg/project.json --script-port 8799 > /tmp/wmsdbg/run4.log 2>&1 < /dev/null &
