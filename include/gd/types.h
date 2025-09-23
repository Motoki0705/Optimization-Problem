#ifndef GD_TYPES_H
#define GD_TYPES_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Objective function type. */
typedef double (*GD_ObjectiveFn)(const double *x, size_t dim, void *ctx);

/** Gradient function type. */
typedef void (*GD_GradientFn)(const double *x, size_t dim, double *grad, void *ctx);

/** Model describing the objective and optional gradient. */
typedef struct {
    size_t dim;
    GD_ObjectiveFn f;
    GD_GradientFn grad;
    void *ctx;
} GD_Model;

/** Optimizer configuration. */
typedef struct {
    double lr;
    double eps;
    size_t max_iter;
    double numgrad_step;
} GD_OptimConfig;

#define GD_DEFAULT_LR 0.1
#define GD_DEFAULT_EPS 1e-2
#define GD_DEFAULT_MAX_ITER 100
#define GD_DEFAULT_NUMGRAD_STEP 1e-6

#ifdef __cplusplus
}
#endif

#endif /* GD_TYPES_H */
