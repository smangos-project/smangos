#!/usr/bin/env bash
# Starts WorldMapStudio into a project, runs its [EditorTest] tests over the script HTTP endpoint,
# prints the results and closes the editor.
# Exit code: 0 all passed, 1 any failed or errored, 2 the run could not be carried out.
set -u

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
wms_dir="$here/WorldMapStudio"
config="$here/wms-project.json"
filter=""; port=0; attach=0; timeout_min=20; no_build=0; core_only=0; json_out=""; keep_open=0

usage() {
    echo "usage: wms-test.sh [--filter F] [--config FILE] [--port N] [--attach] [--timeout-minutes N]"
    echo "                   [--no-build] [--core-only] [--json FILE] [--keep-open]"
}

while [ $# -gt 0 ]; do
    case "$1" in
        --filter) filter="$2"; shift 2 ;;
        --config) config="$2"; shift 2 ;;
        --port) port="$2"; shift 2 ;;
        --attach) attach=1; shift ;;
        --timeout-minutes) timeout_min="$2"; shift 2 ;;
        --no-build) no_build=1; shift ;;
        --core-only) core_only=1; shift ;;
        --json) json_out="$2"; shift 2 ;;
        --keep-open) keep_open=1; shift ;;
        -h|--help) usage; exit 0 ;;
        *) usage; exit 2 ;;
    esac
done

for tool in jq curl python3 dotnet godot-mono; do
    command -v "$tool" >/dev/null || { echo "error: $tool not found"; exit 2; }
done

start=$(date +%s)
deadline=$((start + timeout_min * 60))
launched_pid=""
editor_pid=""
exit_code=2
endpoint=0

say() { printf '[%6ss] %s\n' "$(( $(date +%s) - start ))" "$*"; }

log_path="${XDG_DATA_HOME:-$HOME/.local/share}/godot/app_userdata/WorldMapStudio/logs/godot.log"
log_tail() {
    if [ -f "$log_path" ]; then
        echo "--- tail of $log_path ---"; tail -n 40 "$log_path"; echo "--- end of log ---"
    fi
}

# Godot rewrites the csproj SDK version to its own on import; undo that so the submodule stays clean.
restore_csproj() {
    cd "$wms_dir" || return
    if [ "$(git diff --numstat -- WorldMapStudio.csproj | cut -f1,2)" = "$(printf '1\t1')" ]; then
        git checkout -- WorldMapStudio.csproj
    fi
    rm -f WorldMapStudio.csproj.old
}

alive() { [ -n "$1" ] && kill -0 "$1" 2>/dev/null; }

# Runs JS in the editor; prints the result string, returns 1 on a script error.
wms() {
    local resp
    resp=$(curl -sS --max-time 100 -H 'Content-Type: text/plain; charset=utf-8' \
        --data-binary "$1" "http://127.0.0.1:$endpoint/run") || return 1
    if [ "$(jq -r '.ok' <<<"$resp")" != "true" ]; then
        echo "script error: $(jq -r '.error' <<<"$resp")" >&2
        return 1
    fi
    jq -r '.result' <<<"$resp"
}

health() { curl -sS --max-time 3 "http://127.0.0.1:$endpoint/health" 2>/dev/null; }

check_databases() {
    local cfg="$1" stripped
    stripped=$(grep -v '^\s*//' "$cfg") || return 0
    local base; base=$(dirname "$cfg")
    while IFS= read -r p; do
        [ -z "$p" ] && continue
        case "$p" in /*) f="$p" ;; *) f="$base/$p" ;; esac
        [ -e "$f" ] || { echo "database file not found: $f"; return 1; }
    done < <(jq -r '(.storageLocations // {}) | to_entries[] | .value.databasePath // empty' <<<"$stripped" 2>/dev/null)
}

cleanup() {
    if [ -n "$launched_pid" ] && [ "$keep_open" -eq 0 ]; then
        if alive "$editor_pid"; then
            wms 'wms.editor.Quit(0, true)' >/dev/null 2>&1
            for _ in $(seq 60); do alive "$editor_pid" || break; sleep 0.5; done
            alive "$editor_pid" && say 'the editor did not exit within 30s; killing it'
        fi
        alive "$editor_pid" && kill -9 "$editor_pid" 2>/dev/null
        alive "$launched_pid" && kill -9 "$launched_pid" 2>/dev/null
    fi
}
trap cleanup EXIT

fail() { echo "error: $*"; exit_code=2; if { [ -n "$launched_pid" ] && ! alive "$launched_pid"; } || { [ -n "$editor_pid" ] && ! alive "$editor_pid"; }; then log_tail; fi; exit 2; }

if [ "$attach" -eq 1 ]; then
    endpoint=$([ "$port" -gt 0 ] && echo "$port" || echo 8765)
    h=$(health); [ "$(jq -r '.ready' <<<"${h:-{\}}" 2>/dev/null)" = "true" ] || fail "no ready editor answered on port $endpoint"
    editor_pid=$(jq -r '.pid' <<<"$h")
    say "attached to '$(jq -r '.project' <<<"$h")' (pid $editor_pid, port $endpoint)"
else
    [ -f "$config" ] || fail "project config not found: $config"
    config=$(realpath "$config")
    if [ "$no_build" -eq 0 ]; then
        say 'building'
        cd "$wms_dir" || fail "no $wms_dir"
        if [ "$core_only" -eq 1 ]; then
            out=$(dotnet build WorldMapStudio.csproj -c Debug -p:WmsCoreOnly=true --nologo 2>&1) || { echo "$out"; fail 'dotnet build (WmsCoreOnly) failed'; }
        fi
        out=$(dotnet build --nologo 2>&1) || { echo "$out"; fail 'dotnet build failed'; }
        out=$(godot-mono --headless --import --quit-after 1000 2>&1) || { echo "$out"; fail 'godot import failed'; }
        restore_csproj
    fi
    msg=$(check_databases "$config") || fail "$msg"
    if [ "$port" -gt 0 ]; then endpoint="$port"; else
        endpoint=$(python3 -c 'import socket;s=socket.socket();s.bind(("127.0.0.1",0));print(s.getsockname()[1])')
    fi
    say "starting the editor on port $endpoint"
    (cd "$wms_dir" && setsid godot-mono . -- --project "$config" --script-port "$endpoint" >/tmp/wms-editor.out 2>&1 </dev/null & echo $! >/tmp/wms-editor.pid)
    launched_pid=$(cat /tmp/wms-editor.pid)
    while true; do
        alive "$launched_pid" || fail "the editor exited before it became ready (see /tmp/wms-editor.out)"
        [ "$(date +%s)" -gt "$deadline" ] && fail "the editor was not ready within $timeout_min minutes"
        h=$(health)
        if [ -n "$h" ] && [ "$(jq -r '.ready' <<<"$h" 2>/dev/null)" = "true" ]; then break; fi
        sleep 0.5
    done
    editor_pid=$(jq -r '.pid' <<<"$h")
    say "editor ready: '$(jq -r '.project' <<<"$h")' (pid $editor_pid)"
fi

if [ -z "$filter" ]; then run_call='wms.tests.Run()'; else run_call="wms.tests.Run($(jq -Rn --arg f "$filter" '$f'))"; fi
run_id=$(wms "$run_call") || fail "could not start the run"
say "run $run_id started (${filter:-all tests})"

describe_js='var describe=function(s){return{id:s.Id,state:s.State,isRunning:s.IsRunning,runningId:s.RunningId,total:s.Total,passed:s.Passed,failed:s.Failed,errored:s.Errored,skipped:s.Skipped,notRun:s.NotRun,durationMs:s.DurationMs,allGreen:s.AllGreen};};'
while true; do
    status=$(wms "(async function(){ $describe_js var s = await wms.tests.Wait($run_id, 15000); return JSON.stringify(describe(s)); })()") || fail "lost the editor"
    say "$(jq -r '"\(.passed+.failed+.errored+.skipped)/\(.total) done, \(.passed) passed, \(.failed) failed, \(.errored) errored, \(.skipped) skipped" + (if .runningId then "  running: \(.runningId)" else "" end)' <<<"$status")"
    [ "$(jq -r '.isRunning' <<<"$status")" = "true" ] || break
    if [ "$(date +%s)" -gt "$deadline" ]; then wms "wms.tests.Cancel($run_id)" >/dev/null; fail "the run did not finish within $timeout_min minutes and was cancelled"; fi
done

results_js() {
cat <<JS
(function(){var r=wms.tests.Results($run_id,$1);var out=[];for(var i=0;i<r.length;i++){var t=r[i];var logs=[];for(var j=0;j<t.Logs.length;j++){logs.push(t.Logs[j]);}
out.push({id:t.Id,name:t.Name,category:t.Category,outcome:t.Outcome,durationMs:t.DurationMs,message:t.Message,details:t.Details,logs:logs});}return JSON.stringify(out);})()
JS
}

failures=$(wms "$(results_js true)") || fail "could not read results"
jq -r '
  reduce .[] as $f ({seen:{}, out:[]};
    if ($f.message // "") != "" and .seen[$f.message] then .out += ["FAIL  \($f.id)  [\($f.outcome)]  same as \(.seen[$f.message])"]
    else .seen[$f.message // ""] = $f.id
      | .out += ["", "FAIL  \($f.id)  [\($f.outcome)]"]
      | (if $f.message then .out += ["  \($f.message)"] else . end)
      | (if $f.details then .out += (($f.details | split("\n"))[:6] | map("    " + .)) else . end)
      | .out += ($f.logs | map("  log: " + .))
    end) | .out[]' <<<"$failures"

if [ -n "$json_out" ]; then
    all=$(wms "$(results_js false)") || fail "could not read results"
    jq -n --argjson run "$status" --argjson results "$all" '{run:$run, results:$results}' >"$json_out"
    say "wrote $json_out"
fi

echo
jq -r '"\(.total) tests: \(.passed) passed, \(.failed) failed, \(.errored) errored, \(.skipped) skipped, \(.notRun) not run (\(.durationMs/1000 | . * 10 | floor / 10)s of test time)"' <<<"$status"
if [ "$(jq -r '.allGreen' <<<"$status")" = "true" ]; then echo PASSED; exit_code=0; else echo FAILED; exit_code=1; fi
exit $exit_code
