#include "utils.h"

#include <math.h>

double gd_vec_inf_norm(const double *x, size_t dim) {
    if (!x) {
        return 0.0;
    }
    double max_val = 0.0;
    for (size_t i = 0; i < dim; ++i) {
        const double val = fabs(x[i]);
        if (val > max_val) {
            max_val = val;
        }
    }
    return max_val;
}

void gd_vec_copy(double *dst, const double *src, size_t dim) {
    if (!dst || !src) {
        return;
    }
    for (size_t i = 0; i < dim; ++i) {
        dst[i] = src[i];
    }
}
