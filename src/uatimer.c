/*
 * uatimer.c - Portable policy core for the User-Aware Adaptive Timer.
 *
 * Implements Algorithm 1 and Eqs. (3), (6), (7) of the manuscript. No
 * OS-specific calls appear here; all platform interaction goes through
 * the ua_platform_t callbacks. See include/uatimer.h.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "uatimer.h"

/* Clamp helper. */
static ua_ms_t clip(ua_ms_t v, ua_ms_t lo, ua_ms_t hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void ua_config_default(ua_config_t *cfg)
{
    /* Weights from Table II. Timing values are placeholders and MUST be
     * replaced with platform measurements (see header contract). */
    cfg->lambda = 0.5;
    cfg->alpha  = 0.5;
    cfg->t_hat0 = 1000;   /* 1 s   */
    cfg->t_be   = 50;     /* 50 ms */
    cfg->t_max  = 30000;  /* 30 s  */

    cfg->modes[UA_MODE_ENERGY]   = (ua_mode_cfg_t){0.8, 0.2, 50,  false};
    cfg->modes[UA_MODE_BALANCED] = (ua_mode_cfg_t){0.5, 0.5, 50,  false};
    cfg->modes[UA_MODE_LATENCY]  = (ua_mode_cfg_t){0.2, 0.8, 100, true};
}

double ua_mode_scale(const ua_config_t *cfg, ua_mode_t mode)
{
    const ua_mode_cfg_t *m = &cfg->modes[mode];
    double rho = 1.0 + cfg->alpha * (m->w_d - m->w_e);
    if (rho < 0.0) rho = 0.0;   /* guarantee rho > 0 for alpha < 1 anyway */
    return rho;
}

ua_ms_t ua_effective_threshold(const ua_timer_t *t)
{
    const ua_mode_cfg_t *m = &t->cfg.modes[t->mode];
    double  rho   = ua_mode_scale(&t->cfg, t->mode);
    ua_ms_t scaled = (ua_ms_t)(rho * t->t_hat + 0.5);
    /* The floor is never allowed below the break-even time (Eq. 2). */
    ua_ms_t floor = m->t_min > t->cfg.t_be ? m->t_min : t->cfg.t_be;
    return clip(scaled, floor, t->cfg.t_max);
}

/* Re-arm (or cancel) the one-shot timer for the current mode/estimate. */
static void ua_rearm(ua_timer_t *t, const ua_platform_t *plat)
{
    t->t_eff = ua_effective_threshold(t);

    if (t->cfg.modes[t->mode].inhibit) {
        t->armed = false;
        if (plat && plat->cancel_timer)
            plat->cancel_timer(plat->ctx);
        return;
    }
    t->armed = true;
    if (plat && plat->arm_timer)
        plat->arm_timer(plat->ctx, t->t_last + t->t_eff);
}

void ua_init(ua_timer_t *t, const ua_config_t *cfg, ua_ms_t now_ms)
{
    t->cfg           = *cfg;
    t->t_hat         = (double)cfg->t_hat0;
    t->t_last        = now_ms;
    t->mode          = UA_MODE_BALANCED;
    t->armed         = false;
    t->n_events      = 0;
    t->n_suspends    = 0;
    t->n_inhibited   = 0;
    t->total_sleep_ms = 0;
    t->t_eff         = ua_effective_threshold(t);
}

void ua_on_event(ua_timer_t *t, const ua_platform_t *plat, ua_ms_t now_ms)
{
    ua_ms_t idle = now_ms - t->t_last;
    if (idle < 0) idle = 0;                 /* guard against clock skew */
    t->t_last = now_ms;
    t->n_events++;

    /* Exponential-average update, Eq. (3). */
    t->t_hat = t->cfg.lambda * t->t_hat + (1.0 - t->cfg.lambda) * (double)idle;

    ua_rearm(t, plat);
}

void ua_set_mode(ua_timer_t *t, const ua_platform_t *plat, ua_mode_t mode)
{
    if (mode >= UA_MODE_COUNT) return;
    t->mode = mode;
    ua_rearm(t, plat);
}

bool ua_on_expiry(ua_timer_t *t, const ua_platform_t *plat,
                  ua_ms_t now_ms, bool has_pending)
{
    (void)now_ms;
    /* Do not suspend if inhibited, disarmed, or work is pending. */
    if (has_pending || !t->armed || t->cfg.modes[t->mode].inhibit) {
        t->n_inhibited++;
        return false;
    }
    t->armed = false;
    t->n_suspends++;

    if (plat && plat->suspend) {
        ua_ms_t slept = plat->suspend(plat->ctx);
        if (slept > 0) t->total_sleep_ms += slept;
    }
    return true;
}
