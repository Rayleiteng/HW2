#!/usr/bin/env bash
set -euo pipefail

cc=${CC:-gcc}
cflags=${CFLAGS:--O2 -Wall -Wextra -fopenmp}
ldflags=${LDFLAGS:--lpapi -lpthread -fopenmp}
output=${OUTPUT:-hw1_papi_results.csv}
vector_sizes=(1000 10000 100000 1000000 10000000 100000000)
pi_sizes=(1000 10000 100000 1000000 10000000 100000000 1000000000)
threads=(1 2 4 8 16)
apps=(vector pi)
groups=(base cache icache vecsp vecdp)

if [ -n "${VECTOR_SIZES:-}" ]; then
    read -r -a vector_sizes <<< "$VECTOR_SIZES"
fi

if [ -n "${PI_SIZES:-}" ]; then
    read -r -a pi_sizes <<< "$PI_SIZES"
fi

"$cc" $cflags vector_papi.c -o vector_papi $ldflags
"$cc" $cflags pi_papi.c -o pi_papi $ldflags

echo "app,threads,actual_threads,size,elapsed_seconds,instructions,cycles,l1_i_misses,l1_d_misses,l2_misses,vec_sp,vec_dp" > "$output"

run_group() {
    local app=$1
    local group=$2
    local thread_count=$3
    local size=$4
    local exe="./${app}_papi"

    if row=$("$exe" "$group" "$thread_count" "$size" 2>/dev/null | awk -F, 'NR == 2 { print }'); then
        printf '%s\n' "$row"
    else
        printf ''
    fi
}

for app in "${apps[@]}"; do
    if [ "$app" = "vector" ]; then
        sizes=("${vector_sizes[@]}")
    else
        sizes=("${pi_sizes[@]}")
    fi

    for thread_count in "${threads[@]}"; do
        for size in "${sizes[@]}"; do
            echo "Running app=$app threads=$thread_count size=$size"

            base_row=$(run_group "$app" base "$thread_count" "$size")
            cache_row=$(run_group "$app" cache "$thread_count" "$size")
            icache_row=$(run_group "$app" icache "$thread_count" "$size")
            vecsp_row=$(run_group "$app" vecsp "$thread_count" "$size")
            vecdp_row=$(run_group "$app" vecdp "$thread_count" "$size")

            IFS=, read -r app_name _ _ actual_threads measured_size elapsed instructions cycles _ _ _ _ _ _ <<< "$base_row"
            IFS=, read -r _ _ _ _ _ _ _ _ _ l1_d_misses l2_misses _ _ _ <<< "$cache_row"
            IFS=, read -r _ _ _ _ _ _ _ _ l1_i_misses _ _ _ _ _ <<< "$icache_row"
            IFS=, read -r _ _ _ _ _ _ _ _ _ _ _ vec_sp _ _ <<< "$vecsp_row"
            IFS=, read -r _ _ _ _ _ _ _ _ _ _ _ _ vec_dp _ <<< "$vecdp_row"

            echo "$app_name,$thread_count,$actual_threads,$measured_size,$elapsed,$instructions,$cycles,$l1_i_misses,$l1_d_misses,$l2_misses,$vec_sp,$vec_dp" >> "$output"
        done
    done
done

echo "Saved results to $output"
