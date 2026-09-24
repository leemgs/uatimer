/*
 * uatimer.h - User-Aware Adaptive Timer: portable power-management policy core.
 *
 * This header declares the platform-independent policy described in
 * "UATimer: Lightweight Context-Aware Power Management for Heterogeneous
 * IoT Systems". The policy maintains an exponential-average idle estimate
 * (Eq. 3), scales it per context-selected mode and clips it to a
 * break-even floor and an upper bound (Eqs. 6-7), and exposes an inhibit
 * flag for latency-critical sessions. It contains no OS-specific code: a
 * platform adaptation layer supplies timestamps and calls suspend().
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef UATIMER_H
#define UATIMER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Time is expressed in milliseconds throughout the policy core. */
typedef int64_t ua_ms_t;

/* Context-selected operating modes (Table II of the manuscript). */
typedef enum {
    UA_MODE_ENERGY = 0,   /* w_E=0.8, w_D=0.2  -> shorter threshold */
    UA_MODE_BALANCED,     /* w_E=0.5, w_D=0.5  -> rho = 1          */
    UA_MODE_LATENCY,      /* w_E=0.2, w_D=0.8  -> longer threshold */
    UA_MODE_COUNT
} ua_mode_t;

/* Per-mode weights and floor. The scale rho is derived from the weights
 * as rho = 1 + alpha * (w_D - w_E) so that the mode ordering follows
 * Proposition 1 (delay-heavy modes never pick a shorter threshold). */
typedef struct {
    double  w_e;          /* energy weight, in [0,1]            */
    double  w_d;          /* delay weight, in [0,1]; w_e+w_d==1 */
    ua_ms_t t_min;        /* mode-dependent floor, must be >= t_be */
    bool    inhibit;      /* if true, suspension is suppressed in this mode */
} ua_mode_cfg_t;

/* Static configuration supplied once by the platform. */
typedef struct {
    double        lambda;                 /* smoothing coeff, 0<=lambda<1 (Eq. 3) */
    double        alpha;                  /* mode scaling strength, 0<=alpha<1    */
    ua_ms_t       t_hat0;                 /* initial idle estimate                */
    ua_ms_t       t_be;                   /* platform break-even time (Eq. 1)     */
    ua_ms_t       t_max;                  /* upper bound on the threshold         */
    ua_mode_cfg_t modes[UA_MODE_COUNT];   /* per-mode weights/floor/inhibit       */
} ua_config_t;

/* The full mutable policy state. Fits well under 1 KB. */
typedef struct {
    ua_config_t cfg;
    double      t_hat;        /* current idle estimate (Eq. 3)   */
    ua_ms_t     t_eff;        /* current effective threshold      */
    ua_ms_t     t_last;       /* timestamp of the last event      */
    ua_mode_t   mode;         /* current operating mode           */
    bool        armed;        /* whether a one-shot timer is armed */
    /* Counters, useful for evaluation and inspection. */
    uint64_t    n_events;
    uint64_t    n_suspends;
    uint64_t    n_inhibited;  /* expiries where suspend was suppressed */
    ua_ms_t     total_sleep_ms;
} ua_timer_t;

/* Callbacks the platform provides. Any may be NULL except where noted. */
typedef struct {
    /* Enter the platform low-power state. Returns the actual sleep
     * duration in ms once resumed. Required for suspend to have effect;
     * if NULL, expiries are counted but no sleep is performed. */
    ua_ms_t (*suspend)(void *ctx);
    /* Arm a one-shot wakeup at absolute time deadline_ms (optional). */
    void    (*arm_timer)(void *ctx, ua_ms_t deadline_ms);
    /* Cancel a pending one-shot wakeup (optional). */
    void    (*cancel_timer)(void *ctx);
    void   *ctx;
} ua_platform_t;

/* Return a default configuration matching Table II with placeholder
 * timing parameters. Callers MUST override the timing parameters
 * (lambda, alpha, t_be, t_min, t_max, t_hat0) with values measured on
 * the target platform before use; the defaults are not tuned. */
void ua_config_default(ua_config_t *cfg);

/* Initialise the timer. now_ms is the current monotonic time. */
void ua_init(ua_timer_t *t, const ua_config_t *cfg, ua_ms_t now_ms);

/* Report an application/user/sensor/network event at time now_ms.
 * Updates the idle estimate (Eq. 3) and re-arms the timer. */
void ua_on_event(ua_timer_t *t, const ua_platform_t *plat, ua_ms_t now_ms);

/* Change the operating mode (rule-table lookup lives in the caller). */
void ua_set_mode(ua_timer_t *t, const ua_platform_t *plat, ua_mode_t mode);

/* Handle timer expiry at time now_ms: suspend unless inhibited or work
 * is pending. Returns true if the platform was suspended. has_pending
 * lets the caller veto a transition (non-deferrable work). */
bool ua_on_expiry(ua_timer_t *t, const ua_platform_t *plat,
                  ua_ms_t now_ms, bool has_pending);

/* Recompute t_eff = clip(rho_m * t_hat, t_min^(m), t_max) (Eqs. 6-7). */
ua_ms_t ua_effective_threshold(const ua_timer_t *t);

/* Scale factor rho for a given mode (Eq. 7). Exposed for testing. */
double ua_mode_scale(const ua_config_t *cfg, ua_mode_t mode);

#ifdef __cplusplus
}
#endif

#endif /* UATIMER_H */
