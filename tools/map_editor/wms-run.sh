#!/usr/bin/env bash
# Builds WorldMapStudio and opens it on a project (default: wms-project.json next to this script).
# usage: wms-run.sh [--no-build] [--config FILE] [extra editor args...]
set -eu
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
config="$here/wms-project.json"
build=1
while [ $# -gt 0 ]; do
    case "$1" in
        --no-build) build=0; shift ;;
        --config) config="$2"; shift 2 ;;
        *) break ;;
    esac
done
cd "$here/WorldMapStudio"
if [ "$build" -eq 1 ]; then
    dotnet build --nologo -v q
    godot-mono --headless --import --quit-after 1000
fi
exec godot-mono . -- --project "$(realpath "$config")" "$@"
