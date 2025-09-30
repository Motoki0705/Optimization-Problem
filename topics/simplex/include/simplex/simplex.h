#ifndef SIMPLEX_SIMPLEX_H
#define SIMPLEX_SIMPLEX_H

#include <stddef.h>

typedef enum {
    SIMPLEX_OK = 0,
    SIMPLEX_UNBOUNDED = 1,
    SIMPLEX_INFEASIBLE = 2,
    SIMPLEX_INVALID_INPUT = 3
} SimplexStatus;

SimplexStatus simplex_solve(size_t num_constraints,
                            size_t num_variables,
                            const double *A,
                            const double *b,
                            const double *c,
                            double *out_solution,
                            double *out_optimal);

const char *simplex_status_str(SimplexStatus status);

#endif
