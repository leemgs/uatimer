/*
 * lr_baseline.c - Lightweight linear-regression idle predictor.
 *
 * Reference implementation of the LR baseline described in Section IV-C:
 * it predicts the next idle interval from the previous three idle
 * intervals plus a bias term, using coefficients fitted offline by
 * least squares. It stores four scalar coefficients (< 64 B of model
 * state) and performs one dot product per event. It is intentionally a
 * simple linear predictor, not a TinyML or neural model.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "lr_baseline.h"
#include <string.h>

void lr_init(lr_predictor_t *p, const double coef[4])
{
    memset(p, 0, sizeof(*p));
    if (coef) {
        for (int i = 0; i < 4; i++) p->coef[i] = coef[i];
    } else {
        /* Neutral default: predict the most recent interval. */
        p->coef[0] = 1.0; p->coef[1] = 0.0; p->coef[2] = 0.0; p->coef[3] = 0.0;
    }
}

double lr_predict(const lr_predictor_t *p)
{
    /* coef[0..2] weight the three most recent intervals, coef[3] is bias. */
    double y = p->coef[3]
             + p->coef[0] * p->hist[0]
             + p->coef[1] * p->hist[1]
             + p->coef[2] * p->hist[2];
    return y < 0.0 ? 0.0 : y;
}

void lr_observe(lr_predictor_t *p, double idle_ms)
{
    p->hist[2] = p->hist[1];
    p->hist[1] = p->hist[0];
    p->hist[0] = idle_ms;
}

/* Ordinary least squares fit of y = c0*x0 + c1*x1 + c2*x2 + c3 over the
 * supplied idle sequence, using the normal equations with a small ridge
 * term for numerical stability. Returns 0 on success. This mirrors the
 * "fitted offline by least squares" step; it is provided so the baseline
 * is reproducible, not because fitting happens at run time. */
int lr_fit(lr_predictor_t *p, const double *idle, int n)
{
    if (n < 5) return -1;
    double A[4][4] = {{0}}, b[4] = {0};
    const double ridge = 1e-6;
    for (int t = 3; t < n; t++) {
        double x[4] = { idle[t-1], idle[t-2], idle[t-3], 1.0 };
        double y    = idle[t];
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) A[i][j] += x[i] * x[j];
            b[i] += x[i] * y;
        }
    }
    for (int i = 0; i < 4; i++) A[i][i] += ridge;

    /* Gauss-Jordan elimination on the 4x4 system. */
    for (int col = 0; col < 4; col++) {
        int piv = col;
        for (int r = col + 1; r < 4; r++)
            if ((A[r][col] < 0 ? -A[r][col] : A[r][col]) >
                (A[piv][col] < 0 ? -A[piv][col] : A[piv][col])) piv = r;
        if (A[piv][col] == 0.0) return -2;
        if (piv != col) {
            for (int c = 0; c < 4; c++) { double tmp=A[col][c]; A[col][c]=A[piv][c]; A[piv][c]=tmp; }
            double tmp=b[col]; b[col]=b[piv]; b[piv]=tmp;
        }
        double d = A[col][col];
        for (int c = 0; c < 4; c++) A[col][c] /= d;
        b[col] /= d;
        for (int r = 0; r < 4; r++) {
            if (r == col) continue;
            double f = A[r][col];
            for (int c = 0; c < 4; c++) A[r][c] -= f * A[col][c];
            b[r] -= f * b[col];
        }
    }
    for (int i = 0; i < 4; i++) p->coef[i] = b[i];
    return 0;
}
