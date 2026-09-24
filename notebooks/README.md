# Analysis notebook

`uatimer_analysis.ipynb` turns raw per-run measurements
(`../data/raw_runs.csv`) into the aggregated tables, statistics
(mean, 95% CI, Wilcoxon tests), figures, and LaTeX tables used in the
manuscript.

```sh
pip install nbformat pandas numpy scipy matplotlib jupyter
jupyter notebook notebooks/uatimer_analysis.ipynb
```

The notebook ships with `USE_SYNTHETIC_DEMO = True` so it runs before your
data exists — those numbers are random illustration, **not** measurements.
Set it to `False` once `data/raw_runs.csv` is populated. The notebook
fabricates nothing with the demo flag off.

## Kaggle version

`uatimer_kaggle.ipynb` is a **self-contained** variant that runs on Kaggle
with no repository files, no C compiler, and no internet: it embeds a
pure-Python port of the UATimer policy, the LR baseline, and the trace
replay, plus the full analysis pipeline.

- Upload it via *File → Import Notebook*, then *Run All*.
- To analyse real measurements, add a Kaggle Dataset containing
  `raw_runs.csv`; it is auto-detected under `/kaggle/input/`. Otherwise it
  runs on clearly-labelled synthetic demo data.
- Outputs (figures, LaTeX tables, replay CSV) are written to
  `/kaggle/working/`.
