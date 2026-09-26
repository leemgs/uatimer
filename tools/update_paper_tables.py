#!/usr/bin/env python3
"""Inject freshly computed tables into paper/main.tex.

Reads results/table_results.tex and results/table_reductions.tex (produced by
tools/analyze.py) and replaces the bodies of Table V (tab:results) and
Table VI (tab:derived) in paper/main.tex, and updates the "n=NN" in the
Table V caption.

    python tools/analyze.py data/raw_runs.csv     # regenerate results/*.tex
    python tools/update_paper_tables.py           # inject into main.tex

It only touches the two table bodies and the caption's n; the surrounding
prose (e.g. mentions of "twenty runs") is left for you to check if the run
count changed. Prints a reminder of those spots.

SPDX-License-Identifier: Apache-2.0
"""
import re, os, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TEX = os.path.join(ROOT, "paper", "main.tex")
RES = os.path.join(ROOT, "results", "table_results.tex")
RED = os.path.join(ROOT, "results", "table_reductions.tex")


def body(path, start_marker, stop=r"\bottomrule"):
    """Return the rows of a generated table between the header row's following
    \\midrule (or the start_marker line) and \\bottomrule (exclusive)."""
    lines = open(path).read().splitlines()
    out, started = [], False
    for ln in lines:
        if ln.startswith(start_marker):
            started = True
        if not started:
            continue
        if ln.strip() == stop:
            break
        out.append(ln)
    return "\n".join(out)


def detect_n(path):
    m = re.search(r"n=(\d+)", open(path).read())
    return m.group(1) if m else None


def swap(tex, first_row_prefix, new_body):
    a = tex.index(first_row_prefix)
    b = tex.index(r"\bottomrule", a)
    return tex[:a] + new_body + "\n" + tex[b:]


def main():
    for p in (RES, RED):
        if not os.path.exists(p):
            sys.exit(f"missing {p}; run tools/analyze.py first")

    tex = open(TEX, encoding="utf-8").read()

    # Table V: rows begin at the first data row.
    res_body = body(RES, "Web Browsing & Baseline")
    if "Web Browsing & Baseline" not in tex:
        sys.exit("Table V anchor not found in main.tex")
    tex = swap(tex, "Web Browsing & Baseline", res_body)

    # Table VI: rows begin at "Web browsing &".
    red_body = body(RED, "Web browsing &")
    if "Web browsing &" not in tex:
        sys.exit("Table VI anchor not found in main.tex")
    tex = swap(tex, "Web browsing &", red_body)

    # Caption n=NN
    n = detect_n(RES)
    if n:
        tex = re.sub(r"\(Mean \$\\pm\$ 95\\% CI, \$n=\d+\$\)",
                     rf"(Mean $\\pm$ 95\\% CI, $n={n}$)", tex)

    open(TEX, "w", encoding="utf-8").write(tex)
    print(f"Injected Table V and Table VI (n={n}) into paper/main.tex.")
    print("If the run count changed, also check these prose spots for the old "
          "number:")
    print("  - Methodology: 'repeated twenty times', '$n=20$', Bonferroni "
          "'0.05/15'")
    print("  - Abstract: 'Across twenty runs per condition'")
    print("  - Validity: 'repeated twenty times'")
    print("Then recompile: cd paper && pdflatex -interaction=nonstopmode main.tex")


if __name__ == "__main__":
    main()
