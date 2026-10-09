#!/usr/bin/env bash
# Usage: storage.sh setup|teardown S1|S2|S3|S4|S6  -> prints the directory to run the bench in.
set -euo pipefail
root="$(cd "$(dirname "$0")/../.." && pwd)/.local/charperf"
cmd="${1:?setup|teardown}"; target="${2:?S1|S2|S3|S4|S6}"
case "$target" in
S1) dir="$root/s1";;
S2) dir="$root/s2";;
S3) dir="/mnt/charperf-ext4";;
S4) dir="/mnt/charperf-xfs";;
S6) dir="/dev/shm/charperf";;
*) echo "unknown target $target" >&2; exit 1;;
esac
img_for() { echo "$root/${1}.img"; }
case "$cmd/$target" in
setup/S1|setup/S6) mkdir -p "$dir";;
setup/S2) mkdir -p "$dir"; chattr +C "$dir";;
setup/S3|setup/S4)
  fs=$([ "$target" = S3 ] && echo ext4 || echo xfs); img="$(img_for $target)"
  if ! mountpoint -q "$dir"; then
    mkdir -p "$root/s2"; chattr +C "$root/s2" 2>/dev/null || true
    [ -f "$img" ] || { cp /dev/null "$root/s2/$target.img"; truncate -s 8G "$root/s2/$target.img"; }
    echo "Run as root:" >&2
    echo "  mkdir -p $dir && dev=\$(losetup --direct-io=on -f --show $root/s2/$target.img) && mkfs.$fs -q \$dev && mount \$dev $dir && chown $USER $dir" >&2
    exit 2
  fi;;
teardown/S1|teardown/S2|teardown/S6) rm -rf "$dir";;
teardown/S3|teardown/S4) echo "Run as root: umount $dir && losetup -j $root/s2/$target.img | cut -d: -f1 | xargs losetup -d" >&2;;
esac
echo "$dir"
