#ifndef GD_CALLBACKS_H
#define GD_CALLBACKS_H

#include <stddef.h>

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GD_TrainerState {
    size_t iter;
    double value;
    double grad_norm_inf;
    double *x;
    double *grad;
    const GD_Model *model;
    GD_OptimConfig *config;
    int *stop;
} GD_TrainerState;

typedef void (*GD_Callback)(const GD_TrainerState *state, void *userdata);

typedef struct {
    GD_Callback fn;
    void *userdata;
} GD_CallbackSlot;

void gd_cb_print(const GD_TrainerState *state, void *userdata);

typedef struct {
    double factor;
    size_t period;
    double min_lr;
} GD_LRDecayConfig;

void gd_cb_lr_decay(const GD_TrainerState *state, void *userdata);

typedef struct {
    double target_value;
} GD_EarlyStopConfig;

void gd_cb_early_stop(const GD_TrainerState *state, void *userdata);

#ifdef __cplusplus
}
#endif

#endif /* GD_CALLBACKS_H */
