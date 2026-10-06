#!/usr/bin/env bash
# Builds and installs the extension into a temp Blender profile, then runs the headless tests. Usage: test.sh [--all] [--filter NAME]
set -euo pipefail
here=$(cd "$(dirname "$0")" && pwd)
res=${MODEL_EDITOR_TEST_RES:-/tmp/model-editor-test-res}
python3 "$here/tests/test_registry.py"
python3 "$here/tests/test_install.py"
python3 "$here/tests/test_game_data.py"
rm -rf "$res" /tmp/me-out
"$here/build-addon.sh" --install "$res" >/dev/null
BLENDER_USER_RESOURCES=$res blender -b --factory-startup --python "$here/tests/test_addon.py" -- "$@"
