#ifndef GD_TRAINER_H
#define GD_TRAINER_H

#include "callbacks.h"
#include "loss.h"
#include "optim.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    size_t iterations;
    double final_value;
    double final_grad_norm;
    int converged;
    int stopped_early;
} GD_TrainStats;

GD_TrainStats gd_train_minimize(const GD_Model *model,
                                GD_OptimConfig *config,
                                double *x,
                                const GD_CallbackSlot *callbacks,
                                size_t num_callbacks);

#ifdef __cplusplus
}
#endif

#endif /* GD_TRAINER_H */
