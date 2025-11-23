# plot_speedup.py
import pandas as pd
import matplotlib.pyplot as plt

# Read benchmark results
df = pd.read_csv("results.csv")

# Baseline time per N (use time_std with threads == 1)
baseline = df[df["threads"] == 1][["N", "time_std"]].set_index("N")["time_std"]

def add_speedup_columns(group):
    N = group["N"].iloc[0]
    t_std = baseline.loc[N]
    group = group.copy()
    group["speedup_minmax"] = t_std / group["time_minmax"]
    group["speedup_gnu"] = t_std / group["time_gnu"]
    return group

df = df.groupby("N", as_index=False).apply(add_speedup_columns).reset_index(drop=True)

# ---------- Graph 1: speedup vs number of threads (fixed N >= 1e7) ----------
N_fixed = df["N"].max()  # choose the largest N (should be >= 1e7)
df_fixed = df[df["N"] == N_fixed]

plt.figure()
line1, = plt.plot(
    df_fixed["threads"],
    df_fixed["speedup_minmax"],
    marker="o",
    label="min_max_quicksort"
)
line2, = plt.plot(
    df_fixed["threads"],
    df_fixed["speedup_gnu"],
    marker="o",
    label="__gnu_parallel::sort"
)
baseline_line = plt.axhline(
    1.0,
    linestyle="--",
    label="std::sort baseline"
)

plt.xlabel("Number of threads")
plt.ylabel("Speedup over std::sort")
plt.title(f"Relative speedup vs threads (N = {N_fixed})")
plt.legend(handles=[line1, line2, baseline_line])
plt.grid(True)
plt.tight_layout()
plt.savefig("speedup_vs_threads.png", dpi=200)

# ---------- Graph 2: speedup vs array size (all CPU threads) ----------
max_threads = df["threads"].max()
df_all = df[df["threads"] == max_threads].sort_values("N")

plt.figure()
line1, = plt.plot(
    df_all["N"],
    df_all["speedup_minmax"],
    marker="o",
    label="min_max_quicksort"
)
line2, = plt.plot(
    df_all["N"],
    df_all["speedup_gnu"],
    marker="o",
    label="__gnu_parallel::sort"
)
baseline_line = plt.axhline(
    1.0,
    linestyle="--",
    label="std::sort baseline"
)

plt.xscale("log")  # helpful because sizes differ by orders of magnitude
plt.xlabel("Array size N")
plt.ylabel("Speedup over std::sort")
plt.title(f"Relative speedup vs array size (threads = {max_threads})")
plt.legend(handles=[line1, line2, baseline_line])
plt.grid(True, which="both")
plt.tight_layout()
plt.savefig("speedup_vs_size.png", dpi=200)

print("Saved speedup_vs_threads.png and speedup_vs_size.png")
