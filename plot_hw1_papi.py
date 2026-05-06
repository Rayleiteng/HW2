import argparse
import csv
from pathlib import Path

import matplotlib.pyplot as plt


def read_rows(path):
    rows = []
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append({
                "app": row["app"],
                "threads": int(row["threads"]),
                "size": int(row["size"]),
                "elapsed_seconds": float(row["elapsed_seconds"]),
                "l1_i_misses": int(row["l1_i_misses"]),
                "l1_d_misses": int(row["l1_d_misses"]),
                "l2_misses": int(row["l2_misses"]),
                "instructions": int(row["instructions"]),
                "cycles": int(row["cycles"]),
            })
    return rows


def plot_time_vs_threads(rows, app, size, out):
    subset = [row for row in rows if row["app"] == app and row["size"] == size]
    subset.sort(key=lambda row: row["threads"])

    plt.figure(figsize=(8, 5))
    plt.plot(
        [row["threads"] for row in subset],
        [row["elapsed_seconds"] for row in subset],
        marker="o",
    )
    plt.xscale("log", base=2)
    plt.xlabel("Threads")
    plt.ylabel("Execution time (s)")
    plt.title(f"{app}: execution time vs threads, size={size}")
    plt.grid(True, which="both", linestyle="--", linewidth=0.5)
    plt.tight_layout()
    plt.savefig(out, dpi=200)
    plt.close()


def plot_misses_vs_threads(rows, app, size, out):
    subset = [row for row in rows if row["app"] == app and row["size"] == size]
    subset.sort(key=lambda row: row["threads"])
    threads = [row["threads"] for row in subset]

    plt.figure(figsize=(8, 5))
    plt.plot(threads, [row["l1_i_misses"] for row in subset], marker="o", label="L1-I")
    plt.plot(threads, [row["l1_d_misses"] for row in subset], marker="o", label="L1-D")
    plt.plot(threads, [row["l2_misses"] for row in subset], marker="o", label="L2")
    plt.xscale("log", base=2)
    plt.yscale("log")
    plt.xlabel("Threads")
    plt.ylabel("Cache misses")
    plt.title(f"{app}: cache misses vs threads, size={size}")
    plt.grid(True, which="both", linestyle="--", linewidth=0.5)
    plt.legend()
    plt.tight_layout()
    plt.savefig(out, dpi=200)
    plt.close()


def plot_cycles_vs_threads(rows, app, size, out):
    subset = [row for row in rows if row["app"] == app and row["size"] == size]
    subset.sort(key=lambda row: row["threads"])

    plt.figure(figsize=(8, 5))
    plt.plot(
        [row["threads"] for row in subset],
        [row["cycles"] for row in subset],
        marker="o",
        label="Cycles",
    )
    plt.plot(
        [row["threads"] for row in subset],
        [row["instructions"] for row in subset],
        marker="s",
        label="Instructions",
    )
    plt.xscale("log", base=2)
    plt.yscale("log")
    plt.xlabel("Threads")
    plt.ylabel("Count")
    plt.title(f"{app}: instructions and cycles vs threads, size={size}")
    plt.grid(True, which="both", linestyle="--", linewidth=0.5)
    plt.legend()
    plt.tight_layout()
    plt.savefig(out, dpi=200)
    plt.close()


def main():
    script_dir = Path(__file__).resolve().parent
    parser = argparse.ArgumentParser()
    parser.add_argument("csv", nargs="?", default=str(script_dir / "hw1_papi_results.csv"))
    parser.add_argument("--size", type=int, default=100000000)
    args = parser.parse_args()

    rows = read_rows(args.csv)
    for app in ("vector", "pi"):
        plot_time_vs_threads(rows, app, args.size, script_dir / f"{app}_time_vs_threads.png")
        plot_misses_vs_threads(rows, app, args.size, script_dir / f"{app}_misses_vs_threads.png")
        plot_cycles_vs_threads(rows, app, args.size, script_dir / f"{app}_counts_vs_threads.png")


if __name__ == "__main__":
    main()
