# Measurement data

Put your raw per-run measurements in `raw_runs.csv` (copy the header from
`raw_runs_TEMPLATE.csv`). One row per run:

`platform,workload,policy,run,energy_mj,delay_ms,frame_drop_pct,jitter_ms`

- `policy` ∈ {Baseline, Fixed, LR, UATimer}
- record every individual run (≥10 runs per condition recommended)
- leave `frame_drop_pct` / `jitter_ms` empty except for AR/VR

`raw_runs.csv` is git-ignored so real measurement data is not committed by
accident. The analysis pipeline is `notebooks/uatimer_analysis.ipynb`.
