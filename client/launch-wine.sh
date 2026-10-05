#!/usr/bin/env bash
# Linux counterpart of running custom_launcher.exe: deploys the mingw-built launcher and
# custom.dll next to WowClassic.exe and starts the client through wine.
#
#   client/launch-wine.sh <dir containing WowClassic.exe> [client / custom.dll arguments...]
#
# Env: BUILD_DIR (default build-client, where bin/ lives), WINEPREFIX (default ~/.local/share/classic-client-wine),
#      NO_DEPLOY=1 to skip copying the binaries.
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
bin="${BUILD_DIR:-$root/build-client}/bin"
client="${1:?usage: $0 <dir containing WowClassic.exe> [args...]}"
shift

[[ -f "$client/WowClassic.exe" ]] || { echo "no WowClassic.exe in $client" >&2; exit 1; }

if [[ -z "${NO_DEPLOY:-}" ]]; then
    cp -f "$bin/custom_launcher.exe" "$bin/custom.dll" "$client/"
fi

export WINEPREFIX="${WINEPREFIX:-$HOME/.local/share/classic-client-wine}"
export WINEARCH=win64
export SMANGOS_SERVER_DATA="${SMANGOS_SERVER_DATA:-Z:$root/.server/data}"
export WINEDEBUG="${WINEDEBUG:--all}"
# Without this, mutter lowers the game window behind other windows on click.
wine reg add 'HKCU\Software\Wine\X11 Driver' /v UseTakeFocus /t REG_SZ /d N /f >/dev/null 2>&1 || true

# The launcher queues an APC on the suspended client's main thread, which works under wine as-is.
cd "$client"
exec wine custom_launcher.exe "$@"
