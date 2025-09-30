#ifndef GD_UTILS_H
#define GD_UTILS_H

#include <stddef.h>

double gd_vec_inf_norm(const double *x, size_t dim);
void gd_vec_copy(double *dst, const double *src, size_t dim);

#endif /* GD_UTILS_H */
