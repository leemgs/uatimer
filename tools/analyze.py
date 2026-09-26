#!/usr/bin/env python3
"""UATimer one-shot analysis: raw_runs.csv -> stats, significance, figures, LaTeX.

Self-contained (only pandas/numpy/scipy/matplotlib, all preinstalled on Kaggle).
Works on Kaggle and locally with no other repo files. It computes ONLY from the
supplied measurements and fabricates nothing.

Usage:
    python tools/analyze.py [path/to/raw_runs.csv]

If no path is given it searches /kaggle/input/**, ./data/, and the current dir.
Outputs (figures, LaTeX tables, summary) go to /kaggle/working on Kaggle, else
./results.

SPDX-License-Identifier: Apache-2.0
"""
import os, sys, glob
import numpy as np, pandas as pd
from scipy import stats

POLICY_ORDER = ["Baseline", "Fixed", "LR", "UATimer"]
LABEL = {"Baseline": "Baseline", "Fixed": "Fixed timeout",
         "LR": "LR predictor", "UATimer": "UATimer"}
WORKLOADS = ["Web Browsing", "Video Playback", "AI Inference",
             "AR/VR Gaming", "IoT Sensing"]
DISP = {"Web Browsing": "Web browsing", "Video Playback": "Video playback",
        "AI Inference": "AI inference", "AR/VR Gaming": "AR/VR gaming",
        "IoT Sensing": "IoT sensing"}
CONF = 0.95
REQUIRED = ["platform", "workload", "policy", "run",
            "energy_mj", "delay_ms", "frame_drop_pct", "jitter_ms"]


def find_csv():
    if len(sys.argv) > 1:
        return sys.argv[1]
    for base in ("/kaggle/input", "data", "."):
        hits = glob.glob(os.path.join(base, "**", "raw_runs.csv"), recursive=True)
        if hits:
            return sorted(hits)[0]
    sys.exit("raw_runs.csv not found. Pass its path or place it under data/ or "
             "attach it as a Kaggle dataset.")


def out_dir():
    d = "/kaggle/working" if os.path.isdir("/kaggle") else "results"
    os.makedirs(d, exist_ok=True)
    return d


def agg(df, col):
    g = df.groupby(["workload", "policy"], observed=True)[col].agg(
        ["mean", "std", "count"]).reset_index()
    g["ci95"] = g["count"].apply(
        lambda k: stats.t.ppf(0.5 + CONF / 2, k - 1) if k > 1 else np.nan)
    g["ci95"] *= g["std"] / np.sqrt(g["count"])
    return g.set_index(["workload", "policy"])


def main():
    csv = find_csv()
    out = out_dir()
    df = pd.read_csv(csv)
    miss = set(REQUIRED) - set(df.columns)
    if miss:
        sys.exit(f"raw_runs.csv missing columns: {miss}")
    df["policy"] = pd.Categorical(df["policy"], POLICY_ORDER, ordered=True)
    wls = [w for w in WORKLOADS if w in df["workload"].unique()]
    n = int(df.groupby(["workload", "policy"], observed=True)["run"].count().min())
    print(f"Loaded {csv}: {len(df)} rows, {len(wls)} workloads, "
          f"min {n} runs/cell\n")

    E, D = agg(df, "energy_mj"), agg(df, "delay_ms")
    Fd, Jt = agg(df, "frame_drop_pct"), agg(df, "jitter_ms")

    # ---- reductions with propagated CI ----
    def red(wl, comp):
        a, b = E.loc[(wl, "UATimer")], E.loc[(wl, comp)]
        r = a["mean"] / b["mean"]
        rel = np.hypot(a["ci95"] / a["mean"], b["ci95"] / b["mean"])
        return (1 - r) * 100, r * rel * 100

    # ---- Wilcoxon + Bonferroni ----
    def paired(wl, p):
        return df[(df.workload == wl) & (df.policy == p)].sort_values(
            "run")["energy_mj"].to_numpy()
    comps = [(wl, c) for wl in wls for c in ("Baseline", "Fixed", "LR")]
    m = len(comps)
    thr = 0.05 / m

    print("=== Energy reduction (mean ± 95% CI) and Wilcoxon p (Bonferroni "
          f"threshold {thr:.2e}) ===")
    sig_rows = []
    for wl in wls:
        parts = []
        for c in ("Baseline", "Fixed", "LR"):
            rr, ci = red(wl, c)
            ua, cc = paired(wl, "UATimer"), paired(wl, c)
            k = min(len(ua), len(cc))
            try:
                p = stats.wilcoxon(ua[:k], cc[:k]).pvalue if k >= 3 else np.nan
            except ValueError:
                p = 1.0
            mark = "sig" if (p == p and p < thr) else "ns"
            parts.append(f"{c}:{rr:+.1f}±{ci:.1f}% p={p:.1e}[{mark}]")
            sig_rows.append((wl, c, rr, ci, p, mark))
        print(f"  {wl:15s} " + "  ".join(parts))

    # ---- figures ----
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    mk = {"Baseline": "s", "Fixed": "^", "LR": "D", "UATimer": "o"}
    fig, ax = plt.subplots(figsize=(6, 4.2))
    for wl in wls:
        e0 = E.loc[(wl, "Baseline"), "mean"]
        ax.plot([D.loc[(wl, p), "mean"] for p in POLICY_ORDER],
                [E.loc[(wl, p), "mean"] / e0 for p in POLICY_ORDER],
                color="0.7", lw=.8, zorder=1)
        for p in POLICY_ORDER:
            ax.scatter(D.loc[(wl, p), "mean"], E.loc[(wl, p), "mean"] / e0,
                       marker=mk[p], zorder=2,
                       label=p if wl == wls[0] else None)
    ax.set_xlabel("Mean event delay (ms)"); ax.set_ylabel("Energy / baseline")
    ax.grid(alpha=.3); ax.legend(); fig.tight_layout()
    fig.savefig(f"{out}/fig_energy_delay.png", dpi=150)
    fig.savefig(f"{out}/fig_energy_delay.pdf")

    fig, ax = plt.subplots(figsize=(6.5, 4)); x = np.arange(len(wls)); w = .2
    for i, p in enumerate(POLICY_ORDER):
        ax.bar(x + (i - 1.5) * w, [E.loc[(wl, p), "mean"] for wl in wls], w,
               yerr=[E.loc[(wl, p), "ci95"] for wl in wls], capsize=3, label=p)
    ax.set_xticks(x); ax.set_xticklabels([w.split()[0] for w in wls])
    ax.set_ylabel("Energy (mJ)"); ax.legend(); ax.grid(axis="y", alpha=.3)
    fig.tight_layout(); fig.savefig(f"{out}/fig_energy_bars.png", dpi=150)
    fig.savefig(f"{out}/fig_energy_bars.pdf")

    # ---- LaTeX tables ----
    def cell(S, wl, p, f):
        return f"{f.format(S.loc[(wl, p), 'mean'])}$\\pm${f.format(S.loc[(wl, p), 'ci95'])}"
    L = [f"% auto-generated from {os.path.basename(csv)} (n={n} per cell)",
         r"\begin{tabular}{llrrrr}", r"\toprule",
         r"Workload & Policy & Energy (mJ) & Delay (ms) & Frame drop (\%) & Jitter (ms) \\",
         r"\midrule"]
    for wl in wls:
        for j, p in enumerate(POLICY_ORDER):
            fd = jt = "--"
            if wl == "AR/VR Gaming":
                fd, jt = cell(Fd, wl, p, "{:.2f}"), cell(Jt, wl, p, "{:.2f}")
            nm = wl if j == 0 else ("(60 Hz)" if (wl == "AR/VR Gaming" and j == 1) else "")
            b0, b1 = ("\\textbf{", "}") if p == "UATimer" else ("", "")
            L.append(f"{nm} & {LABEL[p]} & {b0}{cell(E, wl, p, '{:.0f}')}{b1} "
                     f"& {cell(D, wl, p, '{:.1f}')} & {fd} & {jt} \\\\")
        L.append(r"\midrule")
    L[-1] = r"\bottomrule"; L.append(r"\end{tabular}")
    open(f"{out}/table_results.tex", "w").write("\n".join(L))

    L = [f"% auto-generated from {os.path.basename(csv)} (n={n} per cell)",
         r"\begin{tabular}{lrrr}", r"\toprule",
         r"Workload & vs. Baseline & vs. Fixed & vs. LR \\",
         r" & (\%) & (\%) & (\%) \\", r"\midrule"]
    acc = {c: [] for c in ("Baseline", "Fixed", "LR")}
    for wl in wls:
        vals = []
        for c in ("Baseline", "Fixed", "LR"):
            rr, ci = red(wl, c); acc[c].append(rr)
            vals.append(f"{rr:.1f}$\\pm${ci:.1f}")
        L.append(f"{DISP[wl]} & " + " & ".join(vals) + r" \\")
    L.append(r"\midrule")
    L.append("Mean & " + " & ".join(f"{np.mean(acc[c]):.1f}"
             for c in ("Baseline", "Fixed", "LR")) + r" \\")
    L.append(r"\bottomrule"); L.append(r"\end{tabular}")
    open(f"{out}/table_reductions.tex", "w").write("\n".join(L))

    print(f"\nWrote to {out}/:")
    print("  fig_energy_delay.{png,pdf}, fig_energy_bars.{png,pdf}")
    print("  table_results.tex, table_reductions.tex")
    print("\n--- paste-ready Table V (table_results.tex) ---")
    print(open(f"{out}/table_results.tex").read())


if __name__ == "__main__":
    main()
