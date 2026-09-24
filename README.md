# UATimer

Reference implementation of **UATimer**, the user-aware adaptive timer
described in *"UATimer: Lightweight Context-Aware Power Management for
Heterogeneous IoT Systems"* (manuscript in [`paper/`](paper/)).

UATimer is a portable power-management **policy layer**. It keeps an
exponential-average estimate of recent idle intervals, scales that
estimate per a context-selected operating mode, clips it to a
platform break-even floor and an upper bound, and exposes an inhibit
flag for latency-critical sessions. The policy core contains no
OS-specific code; a thin platform layer supplies event timestamps and
performs the actual low-power transition.

## What is and is not in this repository

| Item | Status |
|---|---|
| Portable policy core (Algorithm 1; Eqs. 3, 6, 7) | ✅ `src/uatimer.c`, `include/uatimer.h` |
| Linux `epoll`/`timerfd` backend | ✅ `src/platform_linux.c` |
| Lightweight linear-regression baseline (§IV-C) | ✅ `src/lr_baseline.c` |
| Trace-driven functional harness | ✅ `src/replay.c` |
| Unit tests for the paper's stated properties | ✅ `src/test_uatimer.c` |
| Measurement & analysis pipeline (stats, figures, LaTeX tables) | ✅ `notebooks/uatimer_analysis.ipynb` |
| **Measured parameter values, energy datasets, raw per-run samples** | ❌ author-supplied; see note below |

> **The replay harness models the control logic, not device power.** It
> reports policy behaviour (suspend transitions, profitable vs.
> unprofitable transitions, time asleep) so the algorithm can be run and
> inspected without hardware. It does **not** reproduce the energy (mJ)
> figures in the paper, which come from hardware measurement with a
> Monsoon power monitor and an external meter. The example traces in
> `traces/` and the parameters in `config/` are illustrative shapes, not
> the measured workloads or the tuned parameters.

## Build and run

```sh
make test     # build the core and run the unit tests
make demo     # generate synthetic traces and replay all policies
```

Run the harness on your own trace (one `"<idle_ms> [mode]"` per line;
mode 0=energy, 1=balanced, 2=latency):

```sh
make
./build/uatimer_replay traces/web.trace config/mobile.cfg
```

## Using the policy in an application

```c
#include "uatimer.h"

ua_config_t cfg;
ua_config_default(&cfg);          /* Table II weights + placeholder timing */
cfg.lambda = /* measured */;      /* override timing with platform values  */
cfg.t_be   = /* measured */;      /* break-even time, Eq. (1)              */

ua_timer_t t;
ua_platform_t plat = { my_suspend, my_arm, my_cancel, my_ctx };
ua_init(&t, &cfg, now_ms());

/* on each user/sensor/network event: */
ua_on_event(&t, &plat, now_ms());

/* when the context changes (rule-table lookup lives in your code): */
ua_set_mode(&t, &plat, UA_MODE_LATENCY);

/* when the armed one-shot timer fires: */
ua_on_expiry(&t, &plat, now_ms(), /*has_pending=*/false);
```

On Linux, `ua_linux_run()` in `src/platform_linux.c` wires this up with
`epoll` and `timerfd` for you. On a microcontroller, provide the three
`ua_platform_t` callbacks over the vendor timer and sleep interfaces; the
policy core compiles unchanged.

## Mapping to the manuscript

- **Eq. (3)** exponential-average update → `ua_on_event()`
- **Eqs. (6)–(7)** `T_eff = clip(rho_m * T_hat, T_min, T_max)` → `ua_effective_threshold()`, `ua_mode_scale()`
- **Eq. (1)/(2)** break-even floor → enforced in `ua_effective_threshold()`
- **Table II** operating modes → `ua_config_default()`
- **Algorithm 1** event-driven control → `ua_on_event`/`ua_set_mode`/`ua_on_expiry`
- **Proposition 1** mode ordering → checked in `src/test_uatimer.c`

## Measurement and analysis

`notebooks/uatimer_analysis.ipynb` is the analysis pipeline. It reads raw
per-run measurements from `data/raw_runs.csv` (schema in `data/README.md`)
and produces per-condition means with 95% confidence intervals, energy
reductions with propagated uncertainty, Wilcoxon significance tests, the
energy–delay and per-workload figures, and `\input`-ready LaTeX for the
results/reduction tables. It also includes hardware data-collection
templates (power-meter loop and trace replay).

The notebook ships with `USE_SYNTHETIC_DEMO = True` so it runs before data
exists; those numbers are random illustration, not measurements. Set it to
`False` once `data/raw_runs.csv` is filled.

## Reproducing the paper's measurements

The energy/latency results require the target hardware, the power meters,
and the tuned parameter values, none of which are included here. To
reproduce them: populate `config/` with the measured parameters for each
platform, record the event traces, fill `data/raw_runs.csv` with real
per-run measurements, and run the analysis notebook. See the open items in
`paper/SUBMISSION_CHECKLIST.md`.

## License

Apache-2.0. See [`LICENSE`](LICENSE).
