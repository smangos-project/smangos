#!/usr/bin/env bash
# Usage: run-m1.sh DIR LABEL PLAYERS [extra steady/seed args...]  -> B2 steady runs for full and normal sync, summary appended to .local/charperf/m1/summary.csv
set -euo pipefail
repo="$(cd "$(dirname "$0")/../.." && pwd)"
bench="$repo/build/tools/charstore_bench/charstore_bench"; out="$repo/.local/charperf/m1"
dir="$1"; label="$2"; players="$3"; shift 3
mkdir -p "$out"
for sync in full normal; do
  for changes in 0 5 20; do
    "$bench" seed --schema "$repo/.local/charperf/schema.db" --db "$dir/m1.db" --players "$players" --sync "$sync" "$@" >/dev/null 2>&1
    "$bench" steady --db "$dir/m1.db" --players "$players" --changes "$changes" --epochs 60 --warmup 5 --sync "$sync" \
      --label "$label-$sync-c$changes" --summary "$out/summary.csv" --csv "$out/$label-p$players-$sync-c$changes.csv" "$@" >/dev/null 2>&1
  done
done
rm -f "$dir"/m1.db*
