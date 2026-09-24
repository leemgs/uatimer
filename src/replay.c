/*
 * replay.c - Trace-driven functional harness for UATimer.
 *
 * Replays a trace of inter-event idle intervals through the policy core
 * and reports POLICY-LEVEL behaviour: events, suspend transitions,
 * profitable vs. unprofitable transitions (relative to the break-even
 * time), and total time spent in the low-power state. It also runs the
 * fixed-timeout and LR-predictor policies on the same trace for a
 * mechanism-level comparison.
 *
 * IMPORTANT: this harness is a functional model of the CONTROL LOGIC. It
 * does not model device power and therefore does NOT reproduce the energy
 * (mJ) figures in the manuscript, which come from hardware measurement.
 * Its purpose is to make the algorithm runnable and inspectable and to
 * let the sleep-opportunity behaviour be checked without a device.
 *
 * Trace format (one event per line): "<idle_ms> [mode]"
 *   idle_ms : integer milliseconds since the previous event
 *   mode    : optional 0=energy, 1=balanced, 2=latency (default: keep)
 * Lines beginning with '#' are comments.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "uatimer.h"
#include "lr_baseline.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The suspend callback reads the upcoming sleep duration from ctx. */
typedef struct { ua_ms_t sleep_ms; } sim_ctx_t;
static ua_ms_t sim_suspend(void *ctx) { return ((sim_ctx_t *)ctx)->sleep_ms; }

typedef struct {
    uint64_t events, suspends, profitable, unprofitable;
    ua_ms_t  total_sleep, total_idle;
} stats_t;

/* Run UATimer over the trace. */
static stats_t run_uatimer(const ua_config_t *cfg,
                           const ua_ms_t *idle, const int *mode, int n)
{
    sim_ctx_t sc = { 0 };
    ua_platform_t plat = { sim_suspend, NULL, NULL, &sc };
    ua_timer_t t;
    ua_init(&t, cfg, 0);

    stats_t s;
    memset(&s, 0, sizeof(s));
    ua_ms_t clock = 0;

    for (int i = 0; i < n; i++) {
        if (mode[i] >= 0) ua_set_mode(&t, &plat, (ua_mode_t)mode[i]);
        ua_ms_t d     = idle[i];
        ua_ms_t t_eff = ua_effective_threshold(&t);
        s.total_idle += d;

        /* If the threshold expires before the next event, the device
         * suspends at t_eff and sleeps for the remainder of the idle gap. */
        if (!cfg->modes[t.mode].inhibit && d > t_eff) {
            sc.sleep_ms = d - t_eff;
            if (ua_on_expiry(&t, &plat, clock + t_eff, /*has_pending=*/false)) {
                s.suspends++;
                if (sc.sleep_ms >= cfg->t_be) s.profitable++;
                else                          s.unprofitable++;
                s.total_sleep += sc.sleep_ms;
            }
        }
        clock += d;
        ua_on_event(&t, &plat, clock);   /* Eq. (3) update + re-arm */
        s.events++;
    }
    return s;
}

/* Run a fixed-timeout policy over the same trace for comparison. */
static stats_t run_fixed(ua_ms_t timeout, ua_ms_t t_be,
                         const ua_ms_t *idle, int n)
{
    stats_t s; memset(&s, 0, sizeof(s));
    for (int i = 0; i < n; i++) {
        ua_ms_t d = idle[i];
        s.total_idle += d;
        if (d > timeout) {
            ua_ms_t slept = d - timeout;
            s.suspends++;
            if (slept >= t_be) s.profitable++; else s.unprofitable++;
            s.total_sleep += slept;
        }
        s.events++;
    }
    return s;
}

/* Run the LR predictor: it predicts the next idle, and the device
 * suspends immediately if the prediction exceeds t_be (a standard
 * predictive-shutdown decision). */
static stats_t run_lr(lr_predictor_t *p, ua_ms_t t_be,
                      const ua_ms_t *idle, int n)
{
    stats_t s; memset(&s, 0, sizeof(s));
    for (int i = 0; i < n; i++) {
        ua_ms_t d = idle[i];
        s.total_idle += d;
        double pred = lr_predict(p);
        if (pred >= (double)t_be) {          /* predictive shutdown */
            s.suspends++;
            if (d >= t_be) { s.profitable++;   s.total_sleep += d; }
            else             s.unprofitable++;  /* woke before break-even */
        }
        lr_observe(p, (double)d);
        s.events++;
    }
    return s;
}

static void print_stats(const char *name, const stats_t *s)
{
    double util = s->total_idle ? 100.0 * (double)s->total_sleep / (double)s->total_idle : 0.0;
    printf("%-14s events=%-6llu suspends=%-6llu profitable=%-6llu "
           "unprofitable=%-5llu sleep=%lld/%lld ms (%.1f%% of idle)\n",
           name,
           (unsigned long long)s->events, (unsigned long long)s->suspends,
           (unsigned long long)s->profitable, (unsigned long long)s->unprofitable,
           (long long)s->total_sleep, (long long)s->total_idle, util);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s <trace> [config]\n", argv[0]);
        return 2;
    }
    FILE *f = fopen(argv[1], "r");
    if (!f) { perror("open trace"); return 1; }

    static ua_ms_t idle[1 << 20];
    static int     mode[1 << 20];
    int n = 0;
    char line[256];
    while (fgets(line, sizeof(line), f) && n < (1 << 20)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        long long v; int m = -1;
        int got = sscanf(line, "%lld %d", &v, &m);
        if (got < 1) continue;
        idle[n] = (ua_ms_t)v;
        mode[n] = (got >= 2) ? m : -1;
        n++;
    }
    fclose(f);
    if (n == 0) { fprintf(stderr, "empty trace\n"); return 1; }

    ua_config_t cfg;
    ua_config_default(&cfg);
    /* A config file may override timing parameters, one "key value" per line. */
    if (argc >= 3) {
        FILE *c = fopen(argv[2], "r");
        if (c) {
            char k[64]; double val;
            while (fscanf(c, "%63s %lf", k, &val) == 2) {
                if      (!strcmp(k, "lambda")) cfg.lambda = val;
                else if (!strcmp(k, "alpha"))  cfg.alpha  = val;
                else if (!strcmp(k, "t_hat0")) cfg.t_hat0 = (ua_ms_t)val;
                else if (!strcmp(k, "t_be"))   cfg.t_be   = (ua_ms_t)val;
                else if (!strcmp(k, "t_max"))  cfg.t_max  = (ua_ms_t)val;
            }
            fclose(c);
        }
    }

    printf("trace=%s events=%d lambda=%.3f alpha=%.3f t_be=%lld t_max=%lld\n",
           argv[1], n, cfg.lambda, cfg.alpha,
           (long long)cfg.t_be, (long long)cfg.t_max);

    stats_t su = run_uatimer(&cfg, idle, mode, n);
    stats_t sf = run_fixed(3000, cfg.t_be, idle, n);   /* 3 s fixed timeout */

    lr_predictor_t lr;
    /* Fit the LR model on the trace itself (offline), then replay. */
    static double d[1 << 20];
    for (int i = 0; i < n; i++) d[i] = (double)idle[i];
    lr_init(&lr, NULL);
    lr_fit(&lr, d, n);
    stats_t sl = run_lr(&lr, cfg.t_be, idle, n);

    print_stats("UATimer", &su);
    print_stats("Fixed(3s)", &sf);
    print_stats("LR", &sl);
    return 0;
}
