#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
  echo "Usage: $0 <graph.gr>"
  exit 1
fi

GRAPH_FILE="$1"
if [[ ! -f "$GRAPH_FILE" ]]; then
  echo "Input file not found: $GRAPH_FILE"
  exit 1
fi

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
HTD_BIN="$ROOT_DIR/build/bin/htd_main"
NEW_BIN="$ROOT_DIR/build/bin/htd_min_degree_standalone"

if [[ ! -x "$HTD_BIN" || ! -x "$NEW_BIN" ]]; then
  echo "Binaries missing. Build first:"
  echo "  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release"
  echo "  cmake --build build -j"
  exit 1
fi

RUNS=10
htd_widths=()
htd_times=()
new_widths=()
new_times=()

calc_mean() {
  local arr=("$@")
  awk -v data="${arr[*]}" 'BEGIN{split(data,a," "); s=0; for(i in a){s+=a[i]} printf "%.3f", s/length(a)}'
}

calc_median() {
  local arr=("$@")
  printf "%s\n" "${arr[@]}" | sort -n | awk '{a[NR]=$1} END{ if(NR%2==1){printf "%.3f", a[(NR+1)/2]} else {printf "%.3f", (a[NR/2]+a[NR/2+1])/2} }'
}

echo "Running $RUNS iterations on: $GRAPH_FILE"

for ((i=1; i<=RUNS; ++i)); do
  htd_out="$($HTD_BIN --input gr --instance "$GRAPH_FILE" --output width --strategy min-degree 2>/tmp/htd_run_${i}.err)"
  htd_time_ms="$(/usr/bin/time -f '%e' "$HTD_BIN" --input gr --instance "$GRAPH_FILE" --output width --strategy min-degree >/tmp/htd_run_${i}.out 2>/tmp/htd_time_${i}.txt; cat /tmp/htd_time_${i}.txt | awk '{printf "%.0f", $1*1000}')"

  htd_bag_size="$(echo "$htd_out" | tr -cs '0-9' '\n' | head -n 1)"
  if [[ -z "$htd_bag_size" ]]; then
    echo "Failed to parse htd_main output in run $i"
    echo "Output was: $htd_out"
    exit 1
  fi

  htd_tree_width=$(( htd_bag_size > 0 ? htd_bag_size - 1 : 0 ))
  htd_widths+=("$htd_tree_width")
  htd_times+=("$htd_time_ms")

  new_out="$($NEW_BIN "$GRAPH_FILE")"
  new_tree_width="$(echo "$new_out" | awk -F'=' '/^tree_width=/{print $2}')"
  new_runtime_ms="$(echo "$new_out" | awk -F'=' '/^runtime_ms=/{print $2}')"

  if [[ -z "$new_tree_width" || -z "$new_runtime_ms" ]]; then
    echo "Failed to parse standalone binary output in run $i"
    echo "Output was: $new_out"
    exit 1
  fi

  new_widths+=("$new_tree_width")
  new_times+=("$new_runtime_ms")

done

echo
printf "%-28s %-12s %-12s\n" "Metric" "htd_main" "standalone"
printf "%-28s %-12s %-12s\n" "tree_width mean" "$(calc_mean "${htd_widths[@]}")" "$(calc_mean "${new_widths[@]}")"
printf "%-28s %-12s %-12s\n" "tree_width median" "$(calc_median "${htd_widths[@]}")" "$(calc_median "${new_widths[@]}")"
printf "%-28s %-12s %-12s\n" "runtime_ms mean" "$(calc_mean "${htd_times[@]}")" "$(calc_mean "${new_times[@]}")"
printf "%-28s %-12s %-12s\n" "runtime_ms median" "$(calc_median "${htd_times[@]}")" "$(calc_median "${new_times[@]}")"

echo
echo "htd_main tree_width samples: ${htd_widths[*]}"
echo "htd_main runtime_ms samples: ${htd_times[*]}"
echo "standalone tree_width samples: ${new_widths[*]}"
echo "standalone runtime_ms samples: ${new_times[*]}"
