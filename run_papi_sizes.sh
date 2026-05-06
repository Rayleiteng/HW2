#!/usr/bin/env bash
set -euo pipefail

cc=${CC:-gcc}
cflags=${CFLAGS:--O0 -Wall -Wextra}
ldflags=${LDFLAGS:--lpapi -lpthread}
output=${OUTPUT:-papi_results.csv}

# Replace this list with the exact sizes from the assignment, or pass sizes
# on the command line: ./run_papi_sizes.sh 1000 2000 4000
default_sizes=(1000 10000 100000 1000000 10000000 100000000 1000000000)

if [ "$#" -gt 0 ]; then
    sizes=("$@")
else
    sizes=("${default_sizes[@]}")
fi

"$cc" $cflags papi_test1.c -o papi_test1 $ldflags
"$cc" $cflags papi_test2.c -o papi_test2 $ldflags
"$cc" $cflags papi_test3.c -o papi_test3 $ldflags

echo "size,l1_i_misses,l1_d_misses,l2_misses,llc_misses,instructions,cycles" > "$output"

echo "=== run 1: PAPI_TOT_INS PAPI_TOT_CYC PAPI_L1_DCM PAPI_L2_TCM ==="
echo "=== run 2: PAPI_TOT_INS PAPI_TOT_CYC PAPI_L1_ICM ==="
echo "=== run 3: PAPI_TOT_INS PAPI_TOT_CYC PAPI_L3_TCM ==="
for size in "${sizes[@]}"; do
    run1=$(./papi_test1 "$size" | awk -F, 'NR == 2 { print $2 "," $3 "," $4 "," $5 "," $6 "," $7 }')
    run2=$(./papi_test2 "$size" | awk -F, 'NR == 2 { print $6 }')
    run3=$(./papi_test3 "$size" 2>/dev/null | awk -F, 'NR == 2 { print $6 }' || true)

    IFS=, read -r measured_size hwctrs total_instructions total_cycles l1_dcache_misses l2_total_cache_misses <<< "$run1"
    l1_icache_misses=$run2
    llc_misses=$run3

    echo "$measured_size,$l1_icache_misses,$l1_dcache_misses,$l2_total_cache_misses,$llc_misses,$total_instructions,$total_cycles" >> "$output"
done

echo "Saved results to $output"
