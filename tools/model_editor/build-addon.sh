#!/usr/bin/env bash
# Builds the extension zip into .build/dist. Usage: build-addon.sh [--install DIR] [--dev]
#   --install DIR  install into the Blender user resources at DIR (BLENDER_USER_RESOURCES)
#   --dev          after installing, replace the installed extension dir with a symlink to the source
set -euo pipefail
here=$(cd "$(dirname "$0")" && pwd)
src=$here/addon/wow_model_editor
dist=$here/.build/dist
install="" dev=0
while [ $# -gt 0 ]; do
  case $1 in
    --install) install=$(realpath -m "$2"); shift 2 ;;
    --dev) dev=1; shift ;;
    *) echo "unknown arg $1" >&2; exit 2 ;;
  esac
done

wheel=$(sed -n 's/^wheels = \["\.\/wheels\/\(.*\)"\]/\1/p' "$src/blender_manifest.toml")
[ -n "$wheel" ] || { echo "no wheel in manifest" >&2; exit 1; }
if [ ! -f "$here/.build/wheels/$wheel" ]; then "$here/build-wowlib.sh"; fi
[ -f "$here/.build/wheels/$wheel" ] || { echo "built wheel differs from manifest ($wheel)" >&2; ls "$here/.build/wheels"; exit 1; }
mkdir -p "$src/wheels" "$dist"
rm -f "$src"/wheels/*.whl
cp "$here/.build/wheels/$wheel" "$src/wheels/"

rm -f "$dist"/*.zip
blender -b --factory-startup --command extension build --source-dir "$src" --output-dir "$dist" 
zip=$(ls "$dist"/*.zip)
echo "built $zip"

if [ -n "$install" ]; then
  export BLENDER_USER_RESOURCES=$install
  mkdir -p "$install"
  blender -b --factory-startup --command extension install-file -r user_default -e "$zip"
  if [ $dev = 1 ]; then
    dir=$(ls -d "$install"/extensions/user_default/wow_model_editor)
    rm -rf "$dir"
    ln -s "$src" "$dir"
    echo "dev link $dir -> $src"
  fi
fi
