import argparse
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

def main():
    parser = argparse.ArgumentParser(description="Plot CSV with Seaborn")
    parser.add_argument("--file", required=True, help="Path to CSV file")
    parser.add_argument("--output", help="Output figure to file.")
    args = parser.parse_args()

    # Load data
    df = pd.read_csv(args.file, skipinitialspace=True)

    sns.lineplot(data=df, x=df["lines"], y=df["time"], hue="algorithm", marker='o')
    plt.xscale("log")
    plt.yscale("log")


    plt.xlabel("Lines")
    plt.ylabel("Time (ms)")
    plt.legend()
    plt.grid(True)

    if args.output:
      plt.savefig(args.output)
    else:
      plt.show()

if __name__ == "__main__":
    main()


