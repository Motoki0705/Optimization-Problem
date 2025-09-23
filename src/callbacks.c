#include "gd/callbacks.h"

#include <math.h>
#include <stdio.h>

#include "gd/optim.h"

int gd_csv_logger_open(GD_CSVLogger *logger, const char *path) {
    if (!logger || !path) {
        return 0;
    }
    FILE *fp = fopen(path, "w");
    if (!fp) {
        logger->stream = NULL;
        logger->wrote_header = 0;
        logger->owns_stream = 0;
        return 0;
    }
    logger->stream = fp;
    logger->wrote_header = 0;
    logger->owns_stream = 1;
    return 1;
}

void gd_csv_logger_close(GD_CSVLogger *logger) {
    if (!logger) {
        return;
    }
    if (logger->stream && logger->owns_stream) {
        fclose(logger->stream);
    }
    logger->stream = NULL;
    logger->wrote_header = 0;
    logger->owns_stream = 0;
}

void gd_cb_csv_logger(const GD_TrainerState *state, void *userdata) {
    if (!state || !userdata) {
        return;
    }
    GD_CSVLogger *logger = (GD_CSVLogger *)userdata;
    if (!logger->stream) {
        return;
    }

    if (!logger->wrote_header) {
        fprintf(logger->stream, "iter,value,grad_norm_inf,lr");
        if (state->model && state->model->dim > 0 && state->x) {
            for (size_t i = 0; i < state->model->dim; ++i) {
                fprintf(logger->stream, ",x%zu", i);
            }
        }
        fputc('\n', logger->stream);
        logger->wrote_header = 1;
    }

    double lr = state->config ? state->config->lr : NAN;
    fprintf(logger->stream, "%zu,%.17g,%.17g,%.17g", state->iter, state->value,
            state->grad_norm_inf, lr);
    if (state->model && state->model->dim > 0 && state->x) {
        for (size_t i = 0; i < state->model->dim; ++i) {
            fprintf(logger->stream, ",%.17g", state->x[i]);
        }
    }
    fputc('\n', logger->stream);
    fflush(logger->stream);
}

void gd_cb_print(const GD_TrainerState *state, void *userdata) {
    if (!state) {
        return;
    }
    FILE *out = userdata ? (FILE *)userdata : stdout;
    fprintf(out, "iter=%zu value=%.6f |grad|_inf=%.6f lr=%.6f\n",
            state->iter, state->value, state->grad_norm_inf,
            state->config ? state->config->lr : NAN);
}

void gd_cb_lr_decay(const GD_TrainerState *state, void *userdata) {
    if (!state || !state->config || !userdata) {
        return;
    }
    GD_LRDecayConfig *cfg = (GD_LRDecayConfig *)userdata;
    if (cfg->period == 0) {
        return;
    }
    if (state->iter == 0) {
        return;
    }
    if (state->iter % cfg->period == 0) {
        state->config->lr = fmax(state->config->lr * cfg->factor, cfg->min_lr);
    }
}

void gd_cb_early_stop(const GD_TrainerState *state, void *userdata) {
    if (!state || !state->stop || !userdata) {
        return;
    }
    GD_EarlyStopConfig *cfg = (GD_EarlyStopConfig *)userdata;
    if (state->value <= cfg->target_value) {
        *state->stop = 1;
    }
}
