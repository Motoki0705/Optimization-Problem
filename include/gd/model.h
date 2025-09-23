#ifndef GD_MODEL_H
#define GD_MODEL_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void gd_model_init(GD_Model *model, size_t dim, GD_ObjectiveFn f, GD_GradientFn grad, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* GD_MODEL_H */
