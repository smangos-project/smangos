#!/usr/bin/env bash
# Builds the wowlib abi3 wheel into .build/wheels. Usage: build-wowlib.sh [--tests]
set -euo pipefail
here=$(cd "$(dirname "$0")" && pwd)
build=$here/.build
src=$build/wowlib
pin=ceddf1621581e884763b39ab2f775976c6710103
mkdir -p "$build/bin" "$build/wheels"

# presets and pyproject name the compilers gcc-16/g++-16
for t in gcc g++; do ln -sf "$(command -v $t)" "$build/bin/$t-16"; done
export PATH=$build/bin:$PATH

if [ ! -d "$src/.git" ]; then git clone https://github.com/skarndev/wowlib.git "$src"; fi
git -C "$src" fetch -q origin || true
git -C "$src" checkout -q -f "$pin"
git -C "$src" clean -fdq -e build
for p in "$here"/wowlib-patches/*.patch; do [ -e "$p" ] && git -C "$src" apply "$p"; done

if [ "${1:-}" = "--tests" ]; then
  (cd "$src" && cmake --preset gcc16-release -DWOWLIB_BUILD_PYTHON=OFF && cmake --build build/release && ctest --test-dir build/release --output-on-failure)
fi

cd "$src"
rm -f "$build"/wheels/*.whl
uv build --wheel --python "${PYTHON:-/usr/bin/python3}" --out-dir "$build/wheels" \
  -C cmake.define.WOWLIB_DB_ERAS=tww \
  -C cmake.build-type=Release
ls -la "$build/wheels"
