#!/usr/bin/env bash
# For the vscode task: runs launch-wine.sh in its own session with output going to <client dir>/wine.log, and
# stays alive until the client exits (the client was dying when this wrapper returned right after launching).
# Arguments as for launch-wine.sh. KEEP_RUNNING=1 leaves an already running client alone (for starting a second one).
root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
export WINEPREFIX="${WINEPREFIX:-$HOME/.local/share/classic-client-wine}"
log="${1:?usage: $0 <dir containing WowClassic.exe> [args...]}/wine.log"
# A client left running by an earlier launch (e.g. autologin) shares the wine session and breaks input here.
if [[ -z "${KEEP_RUNNING:-}" ]] && pgrep -f 'WowClassic\.exe' > /dev/null; then
    echo "stopping a running client"
    wineserver -k 2> /dev/null
    sleep 2
fi
echo "launching, wine output in $log (this task ends when the client exits)"
NO_DEPLOY=1 WINEDEBUG="${WINEDEBUG:-err+all,warn+process}" setsid -w "$root/client/launch-wine.sh" "$@" > "$log" 2>&1
# the launcher exits once the client is injected, so wait for the client process itself
sleep 3
while pgrep -f 'WowClassic\.exe' > /dev/null; do sleep 1; done
echo "client exited"
