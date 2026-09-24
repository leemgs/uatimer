/*
 * lr_baseline.h - Lightweight linear-regression idle predictor (baseline).
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LR_BASELINE_H
#define LR_BASELINE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Four scalar coefficients + three-sample history: < 64 B of state. */
typedef struct {
    double coef[4];   /* c0,c1,c2 weight the last three intervals; c3 bias */
    double hist[3];   /* most-recent-first idle intervals (ms)            */
} lr_predictor_t;

void   lr_init(lr_predictor_t *p, const double coef[4]);
double lr_predict(const lr_predictor_t *p);       /* predicted next idle (ms) */
void   lr_observe(lr_predictor_t *p, double idle_ms);
int    lr_fit(lr_predictor_t *p, const double *idle, int n);  /* offline OLS */

#ifdef __cplusplus
}
#endif

#endif /* LR_BASELINE_H */
