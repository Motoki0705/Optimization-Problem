#include <stdio.h>

#include "gd/callbacks.h"


static double quad_objective(const double *x, size_t dim, void *ctx) {
    (void)ctx;
    (void)dim;
    const double diff = x[0] - 3.0;
    return 0.5 * diff * diff;
}

static void quad_gradient(const double *x, size_t dim, double *grad, void *ctx) {
    (void)ctx;
    (void)dim;
    grad[0] = x[0] - 3.0;
}

int main(void) {
    GD_Model model;
    gd_model_init(&model, 1, quad_objective, quad_gradient, NULL);

    GD_OptimConfig cfg = {
        .lr = 0.4,
        .eps = 1e-5,
        .max_iter = 100,
        .numgrad_step = 0.0,
    };

    double x[1] = {2.0};

    GD_CSVLogger logger;
    if (!gd_csv_logger_open(&logger, "gd_quadratic_log.csv")) {
        fprintf(stderr, "failed to open CSV log file\n");
        return 1;
    }

    GD_CallbackSlot callbacks[] = {
        {gd_cb_csv_logger, &logger},

    };

    GD_TrainStats stats = gd_train_minimize(&model, &cfg, x, callbacks, 1);

    gd_csv_logger_close(&logger);


