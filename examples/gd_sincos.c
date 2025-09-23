#include <math.h>
#include <stdio.h>

#include "gd/callbacks.h"

#include "gd/model.h"
#include "gd/trainer.h"

static double sincos_objective(const double *x, size_t dim, void *ctx) {
    (void)ctx;
    if (dim < 2) {
        return 0.0;
    }
    return sin(x[0]) + cos(x[1]);
}

int main(void) {
    GD_Model model;
    gd_model_init(&model, 2, sincos_objective, NULL, NULL);

    GD_OptimConfig cfg = {
        .lr = 0.2,
        .eps = 1e-4,
        .max_iter = 200,
        .numgrad_step = 1e-6,
    };

    double x[2] = {0.0, 0.0};

    GD_LRDecayConfig decay = {
        .factor = 0.5,
        .period = 50,
        .min_lr = 1e-3,
    };

    GD_EarlyStopConfig stop = {
        .target_value = -1.9,
    };

    GD_CSVLogger logger;
    if (!gd_csv_logger_open(&logger, "gd_sincos_log.csv")) {
        fprintf(stderr, "failed to open CSV log file\n");
        return 1;
    }

    GD_CallbackSlot callbacks[] = {
        {gd_cb_csv_logger, &logger},

        {gd_cb_lr_decay, &decay},
        {gd_cb_early_stop, &stop},
    };

    GD_TrainStats stats = gd_train_minimize(&model, &cfg, x, callbacks, 3);

    gd_csv_logger_close(&logger);


    printf("Result: x=(%.6f, %.6f) f=%.6f iterations=%zu converged=%d stopped=%d\n",
           x[0], x[1], stats.final_value, stats.iterations, stats.converged,
           stats.stopped_early);

    return 0;
}
