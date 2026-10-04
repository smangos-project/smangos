#!/bin/sh
# Clean-room check: rebuild .world from pinned inputs and verify it.
# Usage: tools/database_tool/rebuild-check.sh [path/to/database_tool]
set -e
root=$(cd "$(dirname "$0")/../.." && pwd)
tool=${1:-$root/build/tools/database_tool/database_tool}
cd "$root"
rm -rf .world/world.doltlite .world/base.doltlite .world/reference.doltlite
"$tool" create --base-only
"$tool" reference || true
"$tool" create
"$tool" verify --final
