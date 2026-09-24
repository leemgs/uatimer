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
