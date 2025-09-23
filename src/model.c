#include "gd/model.h"

void gd_model_init(GD_Model *model, size_t dim, GD_ObjectiveFn f, GD_GradientFn grad, void *ctx) {
    if (!model) {
        return;
    }
    model->dim = dim;
    model->f = f;
    model->grad = grad;
    model->ctx = ctx;
}
