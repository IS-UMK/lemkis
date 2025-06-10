import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import argparse
from os.path import join
from pathlib import Path

def parse_args():
    parser = argparse.ArgumentParser(
        description="Generate benchmark bar charts per producer/consumer config.")
    parser.add_argument("--csv", "--input", dest="input", type=str, default="results/results.csv",
                        help="Path to the input CSV file.")
    parser.add_argument("--output", type=str, default="results",
                        help="Directory where charts will be saved.")
    return parser.parse_args()

def main():
    args = parse_args()
    source_file = args.input
    output_dir = args.output
    Path(output_dir).mkdir(parents=True, exist_ok=True)

    # Load CSV data
    df = pd.read_csv(source_file)

    # Create a column for configuration: e.g., "1P_1C"
    df["config"] = df["producers"].astype(str) + "P_" + df["consumers"].astype(str) + "C"

    # Sort configs by number of producers and consumers
    def config_key(cfg):
        p, c = cfg.replace("C", "").split("P_")
        return (int(p), int(c))

    configs = sorted(df["config"].unique(), key=config_key)

    # Set Seaborn style
    sns.set(style="whitegrid")

    # Generate a bar plot for each configuration
    for config in configs:
        subset = df[df["config"] == config]
        plt.figure(figsize=(10, 6))
        ax = sns.barplot(data=subset, x="benchmark", y="duration_ms")
        ax.set_yscale("log")
        ax.set_title(f"Benchmark Results for {config}")
        ax.set_xlabel("Structure")
        ax.set_ylabel("Duration (ms)")
        plt.xticks(rotation=45, ha="right")
        plt.tight_layout()
        output_file = join(output_dir, f"benchmark_{config}.png")
        plt.savefig(output_file)
        plt.close()

if __name__ == "__main__":
    main()