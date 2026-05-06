#!/usr/bin/env bash
set -euo pipefail

cc=${CC:-gcc}
cflags=${CFLAGS:--O0 -Wall -Wextra -fopenmp}
ldflags=${LDFLAGS:--lpapi -lpthread -fopenmp}
size=${SIZE:-100000000}
output=${OUTPUT:-parallel_papi_results.csv}
default_sizes=(1000 10000 100000 1000000 10000000 100000000 1000000000)

if [ -n "${SIZES:-}" ]; then
    read -r -a sizes <<< "$SIZES"
else
    sizes=("${default_sizes[@]}")
fi

if [ "$#" -gt 0 ]; then
    threads=("$@")
else
    threads=(1 2 4 8 16)
fi

"$cc" $cflags parallel_papi.c -o parallel_papi $ldflags

echo "threads,actual_threads,size,elapsed_seconds,instructions,cycles,l1_i_misses,l1_d_misses,l2_misses,resource_stalls,no_issue_cycles,branch_correct" > "$output"

for thread_count in "${threads[@]}"; do
    for input_size in "${sizes[@]}"; do
        echo "Running threads=$thread_count size=$input_size"

        base_row=$(./parallel_papi base "$thread_count" "$input_size" | awk -F, 'NR == 2 { print }')
        cache_row=$(./parallel_papi cache "$thread_count" "$input_size" | awk -F, 'NR == 2 { print }')
        icache_row=$(./parallel_papi icache "$thread_count" "$input_size" | awk -F, 'NR == 2 { print }')
        stalls_row=$(./parallel_papi stalls "$thread_count" "$input_size" | awk -F, 'NR == 2 { print }')
        branch_row=$(./parallel_papi branch "$thread_count" "$input_size" | awk -F, 'NR == 2 { print }')

        IFS=, read -r _ threads_requested actual_threads measured_size elapsed instructions cycles _ _ _ _ _ _ <<< "$base_row"
        IFS=, read -r _ _ _ _ _ _ _ _ l1_d_misses l2_misses _ _ _ <<< "$cache_row"
        IFS=, read -r _ _ _ _ _ _ _ l1_i_misses _ _ _ _ _ <<< "$icache_row"
        IFS=, read -r _ _ _ _ _ _ _ _ _ _ resource_stalls no_issue_cycles _ <<< "$stalls_row"
        IFS=, read -r _ _ _ _ _ _ _ _ _ _ _ _ branch_correct <<< "$branch_row"

        echo "$threads_requested,$actual_threads,$measured_size,$elapsed,$instructions,$cycles,$l1_i_misses,$l1_d_misses,$l2_misses,$resource_stalls,$no_issue_cycles,$branch_correct" >> "$output"
    done
done

echo "Saved results to $output"
