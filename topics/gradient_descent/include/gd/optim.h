#ifndef GD_OPTIM_H
#define GD_OPTIM_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void gd_optim_config_defaults(GD_OptimConfig *cfg);
void gd_optim_step(const GD_OptimConfig *cfg, double *x, const double *grad, size_t dim);

#ifdef __cplusplus
}
#endif

#endif /* GD_OPTIM_H */
