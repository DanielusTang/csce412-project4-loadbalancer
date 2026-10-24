#!/usr/bin/env bash
# AggieForge Traffic Cloud - experiment suite (Part 2, requirement 11).
# Every run uses config.txt and overrides one knob with --set; quiet mode;
# summaries land in evidence/. Run from the repository root after `make`.
set -euo pipefail
cd "$(dirname "$0")/.."
OUT=evidence
mkdir -p "$OUT"
BIN=./loadbalancer

run() {            # run <name> [--set key=value ...]
    local name=$1; shift
    "$BIN" --no-prompt --quiet --set log_file="$OUT/$name.txt" "$@" >/dev/null
    printf '%-28s' "$name"
    awk -F': ' '
        /Ending queue size/   {q=$2}
        /Active servers \(end\)/ {split($2,a," "); s=a[1]}
        /Requests rejected/   {split($2,r," "); rej=r[1]}
        /Scale-ups \/ downs/  {sc=$2}
        /Min \/ max servers/  {ms=$2}
        /Avg wait/            {w=$2}
        /Server utilization/  {u=$2}
        END {printf "queue_end=%-6s servers_end=%-3s minmax=%-9s scale=%-9s rejected=%-6s wait=%-14s util=%s\n", q, s, ms, sc, rej, w, u}
    ' "$OUT/$name.txt"
}

echo "== Baseline (deliverable): 20 servers, 100000 cycles, defaults =="
run baseline_20x100000_quiet --servers 20 --cycles 100000

echo; echo "== Arrival rate (arrival_probability) =="
for p in 0.3 0.6 0.9 1.5; do run "arrival_$p" --set arrival_probability=$p; done

echo; echo "== Task length (task_time_max) =="
for t in 20 80 300; do run "tasklen_$t" --set task_time_max=$t; done

echo; echo "== Rest period (cooldown_n) =="
for n in 5 50 500; do run "cooldown_$n" --set cooldown_n=$n; done

echo; echo "== Thresholds (scale_low-scale_high) =="
run thresholds_50-80
run thresholds_70-90 --set scale_low=70 --set scale_high=90
run thresholds_20-120 --set scale_low=20 --set scale_high=120

echo; echo "== Firewall: block half the address space =="
run firewall_half --set blocked_ranges=0.0.0.0/1

echo; echo "== Bonus: switch with two balancers =="
"$BIN" --no-prompt --quiet --set switch_enabled=true --set log_file="$OUT/switch_two_balancers.txt" >/dev/null
grep -E "SWITCH|END|Ending queue|Active servers|Scale-ups" "$OUT/switch_two_balancers.txt" | sed 's/^/  /'

echo; echo "Summaries written to $OUT/"
