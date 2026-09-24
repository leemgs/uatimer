/*
 * test_uatimer.c - Unit tests for the UATimer policy core.
 *
 * Verifies the properties the manuscript states: EWMA convergence
 * (Eq. 4), mode ordering (Proposition 1), the break-even floor (Eq. 2),
 * and the latency-critical inhibit. Exits non-zero on any failure.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "uatimer.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

static int failures = 0;
#define CHECK(cond, msg) do { \
    if (!(cond)) { printf("FAIL: %s\n", msg); failures++; } \
    else         { printf("ok:   %s\n", msg); } } while (0)

int main(void)
{
    ua_config_t cfg;
    ua_config_default(&cfg);
    cfg.lambda = 0.5; cfg.alpha = 0.5; cfg.t_be = 50;
    cfg.t_max = 100000; cfg.t_hat0 = 0;
    for (int m = 0; m < UA_MODE_COUNT; m++) cfg.modes[m].t_min = 50;

    ua_platform_t plat = { NULL, NULL, NULL, NULL };

    /* 1. EWMA convergence: with constant idle t*, t_hat -> t* geometrically. */
    {
        ua_timer_t t; ua_init(&t, &cfg, 0);
        ua_ms_t star = 4000, clk = 0;
        for (int i = 0; i < 40; i++) { clk += star; ua_on_event(&t, &plat, clk); }
        CHECK(fabs(t.t_hat - (double)star) < 1.0, "EWMA converges to t* (Eq. 4)");
    }

    /* 2. Mode ordering: rho_energy < rho_balanced < rho_latency (Prop. 1). */
    {
        double re = ua_mode_scale(&cfg, UA_MODE_ENERGY);
        double rb = ua_mode_scale(&cfg, UA_MODE_BALANCED);
        double rl = ua_mode_scale(&cfg, UA_MODE_LATENCY);
        CHECK(re < rb && rb < rl, "mode scale is monotone in delay weight");
        CHECK(fabs(rb - 1.0) < 1e-9, "balanced mode has rho = 1");
    }

    /* 3. Effective threshold respects the break-even floor (Eq. 2). */
    {
        ua_config_t c2 = cfg; c2.t_be = 200;
        for (int m = 0; m < UA_MODE_COUNT; m++) c2.modes[m].t_min = 10; /* below t_be */
        ua_timer_t t; ua_init(&t, &c2, 0);
        ua_ms_t clk = 0;
        for (int i = 0; i < 20; i++) { clk += 30; ua_on_event(&t, &plat, clk); }
        ua_set_mode(&t, &plat, UA_MODE_ENERGY);
        CHECK(ua_effective_threshold(&t) >= c2.t_be,
              "threshold never drops below break-even time");
    }

    /* 4. Latency-critical mode inhibits suspension. */
    {
        ua_timer_t t; ua_init(&t, &cfg, 0);
        ua_set_mode(&t, &plat, UA_MODE_LATENCY);
        bool slept = ua_on_expiry(&t, &plat, 10000, /*has_pending=*/false);
        CHECK(!slept, "latency-critical mode suppresses suspend");
    }

    /* 5. Ordering carries to thresholds on the same estimate. */
    {
        ua_timer_t t; ua_init(&t, &cfg, 0);
        ua_ms_t clk = 0;
        for (int i = 0; i < 30; i++) { clk += 5000; ua_on_event(&t, &plat, clk); }
        ua_set_mode(&t, &plat, UA_MODE_ENERGY);   ua_ms_t te = ua_effective_threshold(&t);
        ua_set_mode(&t, &plat, UA_MODE_BALANCED); ua_ms_t tb = ua_effective_threshold(&t);
        ua_set_mode(&t, &plat, UA_MODE_LATENCY);  ua_ms_t tl = ua_effective_threshold(&t);
        CHECK(te <= tb && tb <= tl, "threshold nondecreasing with delay weight");
    }

    printf("\n%s (%d failure%s)\n", failures ? "TESTS FAILED" : "ALL TESTS PASSED",
           failures, failures == 1 ? "" : "s");
    return failures ? 1 : 0;
}
