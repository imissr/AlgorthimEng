import os
import pandas as pd
import matplotlib.pyplot as plt

CSV_PATH = os.path.join(os.path.dirname(__file__), "results.csv")

df = pd.read_csv(CSV_PATH)

# ----------------------------
# Detect format and normalize
# ----------------------------
if "N" in df.columns and {"threads", "time_std", "time_minmax", "time_gnu"}.issubset(df.columns):
    # Format 1: N,threads,time_std,time_minmax,time_gnu
    # baseline: threads == 1, time_std per N
    baseline = df[df["threads"] == 1][["N", "time_std"]].drop_duplicates().set_index("N")["time_std"]

    def add_speedup(group):
        # Some pandas versions omit grouping columns in apply; use group.name as fallback.
        N = group["N"].iloc[0] if "N" in group.columns else group.name
        t_std = baseline.loc[N]
        group = group.copy()
        group["speedup_minmax"] = t_std / group["time_minmax"]
        group["speedup_gnu"] = t_std / group["time_gnu"]
        return group

    df = df.groupby("N", as_index=False).apply(add_speedup).reset_index(drop=True)

    # pick largest N for thread scaling
    N_fixed = df["N"].max()
    df_fixed = df[df["N"] == N_fixed].sort_values("threads")

    # pick max threads for size scaling
    max_threads = df["threads"].max()
    df_all = df[df["threads"] == max_threads].sort_values("N")

elif {"mode", "size", "threads", "algo", "time_s"}.issubset(df.columns):
    # Format 2: mode,size,threads,algo,time_s

    # Baseline std_sort time per (mode,size)
    base = df[df["algo"] == "std_sort"][["mode", "size", "time_s"]].rename(columns={"time_s": "t_std"})

    # Merge baseline into all rows for same (mode,size)
    df = df.merge(base, on=["mode", "size"], how="left")

    # Compute speedup for algorithms (std_sort will be 1.0)
    df["speedup"] = df["t_std"] / df["time_s"]

    # Thread sweep plot: mode == "thread"
    df_thread = df[df["mode"] == "thread"].copy()
    if df_thread.empty:
        raise SystemExit("No 'thread' mode data found in CSV.")

    # pick the thread sweep size (usually fixed)
    size_fixed = df_thread["size"].iloc[0]
    df_fixed = df_thread[df_thread["size"] == size_fixed]

    # Pivot to get speedup series per algo
    pivot_fixed = df_fixed.pivot_table(index="threads", columns="algo", values="speedup", aggfunc="median").reset_index()

    # Size sweep plot: mode == "size"
    df_size = df[df["mode"] == "size"].copy()
    if df_size.empty:
        raise SystemExit("No 'size' mode data found in CSV.")

    # pick the threads used in size sweep (should be constant)
    threads_fixed = int(df_size["threads"].mode().iloc[0])
    df_all = df_size[df_size["threads"] == threads_fixed]

    pivot_all = df_all.pivot_table(index="size", columns="algo", values="speedup", aggfunc="median").reset_index()

else:
    raise SystemExit(
        f"Unknown CSV format. Columns are: {list(df.columns)}\n"
        "Expected either:\n"
        "  N,threads,time_std,time_minmax,time_gnu\n"
        "or\n"
        "  mode,size,threads,algo,time_s"
    )

# ----------------------------
# Plot 1: speedup vs threads
# ----------------------------
plt.figure()

if "N" in df.columns:
    # Format 1
    plt.plot(df_fixed["threads"], df_fixed["speedup_minmax"], marker="o", label="min_max_quicksort")
    plt.plot(df_fixed["threads"], df_fixed["speedup_gnu"], marker="o", label="__gnu_parallel::sort")
    plt.axhline(1.0, linestyle="--", label="std::sort baseline")
    plt.title(f"Relative speedup vs threads (N = {N_fixed})")
else:
    # Format 2
    if "min_max" in pivot_fixed.columns:
        plt.plot(pivot_fixed["threads"], pivot_fixed["min_max"], marker="o", label="min_max_quicksort")
    if "gnu_parallel" in pivot_fixed.columns:
        plt.plot(pivot_fixed["threads"], pivot_fixed["gnu_parallel"], marker="o", label="__gnu_parallel::sort")
    plt.axhline(1.0, linestyle="--", label="std::sort baseline")
    plt.title(f"Relative speedup vs threads (size = {size_fixed})")

plt.xlabel("Number of threads")
plt.ylabel("Speedup over std::sort")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("speedup_vs_threads.png", dpi=200)

# ----------------------------
# Plot 2: speedup vs size
# ----------------------------
plt.figure()

if "N" in df.columns:
    plt.plot(df_all["N"], df_all["speedup_minmax"], marker="o", label="min_max_quicksort")
    plt.plot(df_all["N"], df_all["speedup_gnu"], marker="o", label="__gnu_parallel::sort")
    max_threads = df["threads"].max()
    plt.title(f"Relative speedup vs array size (threads = {max_threads})")
    plt.xscale("log")
else:
    if "min_max" in pivot_all.columns:
        plt.plot(pivot_all["size"], pivot_all["min_max"], marker="o", label="min_max_quicksort")
    if "gnu_parallel" in pivot_all.columns:
        plt.plot(pivot_all["size"], pivot_all["gnu_parallel"], marker="o", label="__gnu_parallel::sort")
    plt.title(f"Relative speedup vs array size (threads = {threads_fixed})")
    plt.xscale("log")

plt.axhline(1.0, linestyle="--", label="std::sort baseline")
plt.xlabel("Array size")
plt.ylabel("Speedup over std::sort")
plt.grid(True, which="both")
plt.legend()
plt.tight_layout()
plt.savefig("speedup_vs_size.png", dpi=200)

print("Saved speedup_vs_threads.png and speedup_vs_size.png")
