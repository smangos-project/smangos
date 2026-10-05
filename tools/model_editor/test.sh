#!/usr/bin/env bash
# Builds and installs the extension into a temp Blender profile, then runs the headless tests. Usage: test.sh [--all] [--filter NAME]
set -euo pipefail
here=$(cd "$(dirname "$0")" && pwd)
res=${MODEL_EDITOR_TEST_RES:-/tmp/model-editor-test-res}
rm -rf "$res"
"$here/build-addon.sh" --install "$res" >/dev/null
BLENDER_USER_RESOURCES=$res blender -b --factory-startup --python "$here/tests/test_addon.py" -- "$@"
