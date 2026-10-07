#!/bin/bash
pkill -f "godot-mono . -- --project /tmp/wmsdbg"
rm -f /tmp/wmsdbg/world.doltlite.wms-lock
