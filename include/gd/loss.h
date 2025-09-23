#ifndef GD_LOSS_H
#define GD_LOSS_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

double gd_loss_value(const GD_Model *model, const double *x);
void gd_loss_gradient(const GD_Model *model, const double *x, double step, double *grad);

#ifdef __cplusplus
}
#endif

#endif /* GD_LOSS_H */
