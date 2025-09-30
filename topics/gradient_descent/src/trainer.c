#include "gd/trainer.h"

#include <stdlib.h>

#include "utils.h"

GD_TrainStats gd_train_minimize(const GD_Model *model,
                                GD_OptimConfig *config,
                                double *x,
                                const GD_CallbackSlot *callbacks,
                                size_t num_callbacks) {
    GD_TrainStats stats = {0};
    if (!model || !model->f || !config || !x) {
        return stats;
    }

    gd_optim_config_defaults(config);

    double *grad = (double *)malloc(model->dim * sizeof(double));
    if (!grad) {
        return stats;
    }

    int stop_flag = 0;

    for (size_t iter = 0; iter < config->max_iter; ++iter) {
        const double value = gd_loss_value(model, x);
        gd_loss_gradient(model, x, config->numgrad_step, grad);
        const double grad_norm = gd_vec_inf_norm(grad, model->dim);

        stats.iterations = iter + 1;
        stats.final_value = value;
        stats.final_grad_norm = grad_norm;

        GD_TrainerState state = {
            .iter = iter,
            .value = value,
            .grad_norm_inf = grad_norm,
            .x = x,
            .grad = grad,
            .model = model,
            .config = config,
            .stop = &stop_flag,
        };

        for (size_t i = 0; callbacks && i < num_callbacks; ++i) {
            if (callbacks[i].fn) {
                callbacks[i].fn(&state, callbacks[i].userdata);
            }
        }

        if (stop_flag) {
            stats.stopped_early = 1;
            break;
        }

        if (grad_norm < config->eps) {
            stats.converged = 1;
            break;
        }

        gd_optim_step(config, x, grad, model->dim);
    }

    free(grad);
    return stats;
}
