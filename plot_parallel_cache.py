import argparse
import csv
from pathlib import Path

import matplotlib.pyplot as plt


def read_results(path):
    rows = []
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append({
                "threads": int(row["threads"]),
                "size": int(row["size"]),
                "elapsed_seconds": float(row["elapsed_seconds"]),
                "cycles": int(row["cycles"]),
                "l1_i_misses": int(row["l1_i_misses"]),
                "l1_d_misses": int(row["l1_d_misses"]),
                "l2_misses": int(row["l2_misses"]),
                "resource_stalls": int(row["resource_stalls"]),
                "no_issue_cycles": int(row["no_issue_cycles"]),
            })
    return rows


def plot_misses(rows, output):
    thread_counts = sorted({row["threads"] for row in rows})

    plt.figure(figsize=(8, 5))
    for thread_count in thread_counts:
        subset = [row for row in rows if row["threads"] == thread_count]
        subset.sort(key=lambda row: row["size"])
        plt.plot(
            [row["size"] for row in subset],
            [row["l1_d_misses"] for row in subset],
            marker="o",
            label=f"L1-D, {thread_count} threads",
        )
        plt.plot(
            [row["size"] for row in subset],
            [row["l2_misses"] for row in subset],
            marker="s",
            linestyle="--",
            label=f"L2, {thread_count} threads",
        )
    plt.xscale("log")
    plt.yscale("log")
    plt.xlabel("Input size")
    plt.ylabel("Misses")
    plt.title("Parallel cache misses vs input size")
    plt.grid(True, which="both", linestyle="--", linewidth=0.5)
    plt.legend()
    plt.tight_layout()
    plt.savefig(output, dpi=200)
    plt.close()


def plot_time_stalls(rows, output):
    largest_size = max(row["size"] for row in rows)
    subset = [row for row in rows if row["size"] == largest_size]
    subset.sort(key=lambda row: row["threads"])
    threads = [row["threads"] for row in subset]

    fig, ax1 = plt.subplots(figsize=(8, 5))
    ax1.plot(threads, [row["elapsed_seconds"] for row in subset], marker="o", color="tab:blue", label="Elapsed time")
    ax1.set_xscale("log", base=2)
    ax1.set_xlabel("Number of threads")
    ax1.set_ylabel("Elapsed time (s)", color="tab:blue")
    ax1.tick_params(axis="y", labelcolor="tab:blue")
    ax1.grid(True, which="both", linestyle="--", linewidth=0.5)

    ax2 = ax1.twinx()
    ax2.plot(threads, [row["resource_stalls"] for row in subset], marker="s", color="tab:red", label="Resource stalls")
    ax2.set_ylabel("Resource stalls", color="tab:red")
    ax2.tick_params(axis="y", labelcolor="tab:red")

    plt.title(f"Execution time and resource stalls vs threads, size={largest_size}")
    fig.tight_layout()
    plt.savefig(output, dpi=200)
    plt.close()


def main():
    script_dir = Path(__file__).resolve().parent
    parser = argparse.ArgumentParser()
    parser.add_argument("csv", nargs="?", default=str(script_dir / "parallel_papi_results.csv"))
    parser.add_argument("--misses-out", default=str(script_dir / "parallel_cache_misses.png"))
    parser.add_argument("--time-stalls-out", default=str(script_dir / "parallel_time_stalls.png"))
    args = parser.parse_args()

    rows = read_results(args.csv)
    plot_misses(rows, args.misses_out)
    plot_time_stalls(rows, args.time_stalls_out)


if __name__ == "__main__":
    main()
