#include "gd/callbacks.h"

#include <math.h>
#include <stdio.h>

#include "gd/optim.h"

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
