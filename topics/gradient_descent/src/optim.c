#include "gd/optim.h"

#include <stddef.h>

void gd_optim_config_defaults(GD_OptimConfig *cfg) {
    if (!cfg) {
        return;
    }
    if (cfg->lr <= 0.0) {
        cfg->lr = GD_DEFAULT_LR;
    }
    if (cfg->eps <= 0.0) {
        cfg->eps = GD_DEFAULT_EPS;
    }
    if (cfg->max_iter == 0) {
        cfg->max_iter = GD_DEFAULT_MAX_ITER;
    }
    if (cfg->numgrad_step <= 0.0) {
        cfg->numgrad_step = GD_DEFAULT_NUMGRAD_STEP;
    }
}

void gd_optim_step(const GD_OptimConfig *cfg, double *x, const double *grad, size_t dim) {
    if (!cfg || !x || !grad) {
        return;
    }
    for (size_t i = 0; i < dim; ++i) {
        x[i] -= cfg->lr * grad[i];
    }
}
