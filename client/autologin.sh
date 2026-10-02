#!/usr/bin/env bash
# Starts the client logged in and in the world, without touching the login screen:
# gets a login ticket from the server's REST portal, hands it to the client through
# custom_launcher's --webtoken (-launcherlogin), and presses Enter on the character
# screen until the server reports the character online.
#
#   client/autologin.sh [--restart-server] [ACCOUNT [PASSWORD [CHARACTER]]]
#
# Defaults: CLAUDE / claude, the account's last played character. --restart-server
# kills server and client and starts build/src/server/server with its console on the
# fifo /tmp/srv.in (output in /tmp/srv.out). Exit status 0 once in the world; a
# screenshot is saved to /tmp/autologin.jpg either way.
#
# Env: CLIENT_DIR (default .client/_classic_era_), CONTROL_PORT (8790), PORTAL (127.0.0.1:8081),
#      DB ("-h127.0.0.1 -umangos -pmangos"), TIMEOUT (seconds in-world wait, 120).
set -uo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$root"

restart=0
if [[ "${1:-}" == "--restart-server" ]]; then
    restart=1
    shift
fi
account="${1:-CLAUDE}"
password="${2:-claude}"
character="${3:-}"

client_dir="${CLIENT_DIR:-$root/.client/_classic_era_}"
control="localhost:${CONTROL_PORT:-8790}"
portal="${PORTAL:-127.0.0.1:8081}"
db="${DB:--h127.0.0.1 -umangos -pmangos}"
timeout="${TIMEOUT:-120}"

log() { echo "autologin: $*" >&2; }
sql() { mariadb $db -N -B -e "$1" 2>/dev/null; }

quit_client() {
    curl -s -m 5 -X POST "$control/quit" >/dev/null 2>&1 || return 0
    for _ in $(seq 20); do
        curl -s -m 2 "$control/status" >/dev/null 2>&1 || break
        sleep 1
    done
    sleep 2
}

screenshot() { curl -s -m 20 "$control/screenshot?save=Z:/tmp/autologin.jpg" >/dev/null 2>&1; }

quit_client

if ((restart)); then
    pkill -f 'tail -f /tmp/srv.in'
    pkill -f 'build/src/server/server'
    for _ in $(seq 30); do
        pgrep -f 'build/src/server/server' >/dev/null || break
        sleep 1
    done
    rm -f /tmp/srv.in
    mkfifo /tmp/srv.in
    (tail -f /tmp/srv.in | build/src/server/server >/tmp/srv.out 2>&1 &) </dev/null >/dev/null 2>&1
    log "server started"
fi

account_upper="${account^^}"
account_id=$(sql "SELECT id FROM realmd.account WHERE username = '$account_upper'")
[[ -n "$account_id" ]] || { log "no account $account_upper"; exit 1; }

# The realm list only preselects a realm (and skips the realm screen) once the realm is online.
ticket=""
for _ in $(seq 120); do
    flags=$(sql "SELECT realmflags FROM realmd.realmlist WHERE id = 1")
    if [[ -n "$flags" ]] && (((flags & 2) == 0)); then
        ticket=$(curl -sk -m 5 -X POST "https://$portal/bnetserver/login/" \
            -d "{\"inputs\":[{\"input_id\":\"account_name\",\"value\":\"$account\"},{\"input_id\":\"password\",\"value\":\"$password\"}]}" |
            sed -n 's/.*"login_ticket":"\([^"]*\)".*/\1/p')
        [[ -n "$ticket" ]] && break
    fi
    sleep 1
done
[[ -n "$ticket" ]] || { log "no login ticket (server down, realm offline or wrong password)"; exit 1; }

world_port=$(sql "SELECT port FROM realmd.realmlist WHERE id = 1")
online_query="SELECT COUNT(*) FROM characters.characters WHERE account = $account_id AND online = 1"

# A character of a client that just quit stays online until the server drops its session.
for _ in $(seq 60); do
    [[ "$(sql "$online_query")" == "0" ]] && break
    sleep 1
done

if [[ -n "$character" ]]; then
    row=$(sql "SELECT guid, name FROM characters.characters WHERE account = $account_id AND name = '$character'")
    [[ -n "$row" ]] || { log "no character $character on $account_upper"; exit 1; }
    read -r guid name <<<"$row"
    sql "REPLACE INTO realmd.account_last_played_character (accountId, region, battlegroup, realmId, characterName, characterGuid, lastPlayedTime)
         VALUES ($account_id, 1, 1, 1, '$name', $guid, UNIX_TIMESTAMP())"
fi

# Launcher login reads its portal from CONNECTION_STRING, not the portal cvar.
WINEPREFIX="${WINEPREFIX:-$HOME/.local/share/classic-client-wine}" WINEDEBUG=-all \
    wine reg add 'HKCU\Software\Custom Game Server Dev\Battle.net\Launch Options\WoW' \
    /v CONNECTION_STRING /t REG_SZ /d "${portal%%:*}" /f >/dev/null 2>&1

world_connected() { [[ -n "$(ss -Htn state established "( dport = :$world_port )" 2>/dev/null)" ]]; }

# Launches the client and waits for the launcher login to reach the realm. Now and then the
# client sits on an empty login screen without ever connecting; the caller then relaunches it.
launch_client() {
    setsid "$root/client/launch-wine.sh" "$client_dir" --control="${CONTROL_PORT:-8790}" --webtoken="$ticket" \
        >/tmp/client.out 2>&1 </dev/null &
    for _ in $(seq 90); do
        curl -s -m 2 "$control/status" 2>/dev/null | grep -q '"window":true' && break
        sleep 2
    done
    log "client window up"
    for _ in $(seq 40); do
        world_connected && return 0
        sleep 1
    done
    return 1
}

for attempt in 1 2 3; do
    launch_client && break
    log "no realm connection after launch (attempt $attempt)"
    quit_client
done

deadline=$((SECONDS + timeout))
while ((SECONDS < deadline)); do
    if [[ "$(sql "$online_query")" != "0" ]]; then
        sleep 5 # let the world finish loading
        screenshot
        log "in world"
        exit 0
    fi
    # Enter during "Connecting" cancels the login, so only press it on the character screen,
    # where the client holds a world connection.
    if world_connected; then
        sleep 2
        curl -s -m 5 -X POST "$control/key" -d '{"keys":"enter"}' >/dev/null 2>&1
        sleep 2
    else
        sleep 1
    fi
done

screenshot
log "not in world after ${timeout}s, see /tmp/autologin.jpg"
exit 1
