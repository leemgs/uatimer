# Measurement data

Put your raw per-run measurements in `raw_runs.csv` (copy the header from
`raw_runs_TEMPLATE.csv`). One row per run:

`platform,workload,policy,run,energy_mj,delay_ms,frame_drop_pct,jitter_ms`

- `policy` ∈ {Baseline, Fixed, LR, UATimer}
- record every individual run (≥10 runs per condition recommended)
- leave `frame_drop_pct` / `jitter_ms` empty except for AR/VR

`raw_runs.csv` is git-ignored so real measurement data is not committed by
accident. The analysis pipeline is `notebooks/uatimer_analysis.ipynb`.

## Example file

`raw_runs_expected.csv` is a **filled-in example** showing the exact
layout the pipeline expects: 5 workloads × 4 policies × 10 runs = 200 rows,
with `frame_drop_pct` / `jitter_ms` populated only for AR/VR.

> ⚠️ The numbers in `raw_runs_expected.csv` are **illustrative synthetic
> values, not measurements.** Use it only as a template for column order,
> naming, run counts, and which cells carry QoE columns. To try the
> pipeline end-to-end, copy it to `raw_runs.csv`:
>
> ```sh
> cp data/raw_runs_expected.csv data/raw_runs.csv
> ```
>
> Then replace every value with your real per-run measurements before
> using any result in the paper.
