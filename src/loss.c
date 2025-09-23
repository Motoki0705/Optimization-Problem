#include "gd/loss.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

static void gd_numeric_gradient(const GD_Model *model, const double *x, double step, double *grad) {
    if (!model || !model->f || !grad) {
        return;
    }
    const size_t dim = model->dim;
    if (dim == 0) {
        return;
    }
    if (step <= 0.0) {
        step = GD_DEFAULT_NUMGRAD_STEP;
    }
    double *x_tmp = (double *)malloc(dim * sizeof(double));
    if (!x_tmp) {
        return;
    }
    memcpy(x_tmp, x, dim * sizeof(double));
    for (size_t i = 0; i < dim; ++i) {
        const double orig = x_tmp[i];
        x_tmp[i] = orig + step;
        const double forward = model->f(x_tmp, dim, model->ctx);
        x_tmp[i] = orig - step;
        const double backward = model->f(x_tmp, dim, model->ctx);
        grad[i] = (forward - backward) / (2.0 * step);
        x_tmp[i] = orig;
    }
    free(x_tmp);
}

double gd_loss_value(const GD_Model *model, const double *x) {
    if (!model || !model->f || !x) {
        return NAN;
    }
    return model->f(x, model->dim, model->ctx);
}

void gd_loss_gradient(const GD_Model *model, const double *x, double step, double *grad) {
    if (!model || !x || !grad) {
        return;
    }
    if (model->grad) {
        model->grad(x, model->dim, grad, model->ctx);
        return;
    }
    gd_numeric_gradient(model, x, step, grad);
}
