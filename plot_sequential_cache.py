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
                "size": int(row["size"]),
                "l1_i_misses": int(row["l1_i_misses"]),
                "l1_d_misses": int(row["l1_d_misses"]),
                "l2_misses": int(row["l2_misses"]),
                "instructions": int(row["instructions"]),
                "cycles": int(row["cycles"]),
            })
    return rows


def plot_cache_misses(rows, output):
    sizes = [row["size"] for row in rows]

    plt.figure(figsize=(8, 5))
    plt.plot(sizes, [row["l1_i_misses"] for row in rows], marker="o", label="L1-I misses")
    plt.plot(sizes, [row["l1_d_misses"] for row in rows], marker="o", label="L1-D misses")
    plt.plot(sizes, [row["l2_misses"] for row in rows], marker="o", label="L2 misses")
    plt.xscale("log")
    plt.yscale("log")
    plt.xlabel("Input size")
    plt.ylabel("Misses")
    plt.title("Cache misses vs input size")
    plt.grid(True, which="both", linestyle="--", linewidth=0.5)
    plt.legend()
    plt.tight_layout()
    plt.savefig(output, dpi=200)
    plt.close()


def plot_instructions_cycles(rows, output):
    sizes = [row["size"] for row in rows]

    plt.figure(figsize=(8, 5))
    plt.plot(sizes, [row["instructions"] for row in rows], marker="o", label="Instructions")
    plt.plot(sizes, [row["cycles"] for row in rows], marker="o", label="Cycles")
    plt.xscale("log")
    plt.yscale("log")
    plt.xlabel("Input size")
    plt.ylabel("Count")
    plt.title("Instructions and cycles vs input size")
    plt.grid(True, which="both", linestyle="--", linewidth=0.5)
    plt.legend()
    plt.tight_layout()
    plt.savefig(output, dpi=200)
    plt.close()


def main():
    script_dir = Path(__file__).resolve().parent

    parser = argparse.ArgumentParser()
    parser.add_argument("csv", nargs="?", default=str(script_dir / "papi_results.csv"))
    parser.add_argument("--misses-out", default=str(script_dir / "cache_misses.png"))
    parser.add_argument("--instr-cycles-out", default=str(script_dir / "instructions_cycles.png"))
    args = parser.parse_args()

    rows = read_results(args.csv)
    plot_cache_misses(rows, args.misses_out)
    plot_instructions_cycles(rows, args.instr_cycles_out)


if __name__ == "__main__":
    main()
