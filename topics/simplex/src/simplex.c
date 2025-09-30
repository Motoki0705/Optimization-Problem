#include "simplex/simplex.h"

#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static const double SIMPLEX_EPS = 1e-9;

const char *simplex_status_str(SimplexStatus status) {
    switch (status) {
        case SIMPLEX_OK:
            return "optimal";
        case SIMPLEX_UNBOUNDED:
            return "unbounded";
        case SIMPLEX_INFEASIBLE:
            return "infeasible";
        case SIMPLEX_INVALID_INPUT:
        default:
            return "invalid_input";
    }
}

SimplexStatus simplex_solve(size_t num_constraints,
                            size_t num_variables,
                            const double *A,
                            const double *b,
                            const double *c,
                            double *out_solution,
                            double *out_optimal) {
    if (num_constraints == 0 || num_variables == 0 || !A || !b || !c || !out_solution || !out_optimal) {
        return SIMPLEX_INVALID_INPUT;
    }

    for (size_t i = 0; i < num_constraints; ++i) {
        if (b[i] < -SIMPLEX_EPS) {
            return SIMPLEX_INFEASIBLE; // We only support Ax <= b with non-negative b
        }
    }

    const size_t width = num_variables + num_constraints + 1; // variables + slacks + RHS
    const size_t height = num_constraints + 1;                 // constraints + objective
    double *tableau = (double *)calloc(height * width, sizeof(double));
    if (!tableau) {
        return SIMPLEX_INVALID_INPUT;
    }

    size_t *basis = (size_t *)malloc(num_constraints * sizeof(size_t));
    if (!basis) {
        free(tableau);
        return SIMPLEX_INVALID_INPUT;
    }

    // Constraints
    for (size_t i = 0; i < num_constraints; ++i) {
        size_t row = i * width;
        memcpy(&tableau[row], &A[i * num_variables], num_variables * sizeof(double));
        tableau[row + num_variables + i] = 1.0; // slack variable
        tableau[row + width - 1] = b[i];
        basis[i] = num_variables + i;
    }

    // Objective row (maximization -> use negative coefficients)
    size_t obj_row = num_constraints * width;
    for (size_t j = 0; j < num_variables; ++j) {
        tableau[obj_row + j] = -c[j];
    }

    SimplexStatus status = SIMPLEX_OK;

    while (true) {
        // Choose entering column (most negative coefficient in objective row)
        double most_negative = 0.0;
        size_t pivot_col = width; // sentinel invalid index
        for (size_t j = 0; j < width - 1; ++j) {
            double coeff = tableau[obj_row + j];
            if (coeff < most_negative - SIMPLEX_EPS) {
                most_negative = coeff;
                pivot_col = j;
            }
        }

        if (pivot_col == width) {
            break; // optimal
        }

        // Determine leaving row using minimum ratio test
        double best_ratio = DBL_MAX;
        size_t pivot_row = height; // sentinel
        for (size_t i = 0; i < num_constraints; ++i) {
            double coeff = tableau[i * width + pivot_col];
            if (coeff > SIMPLEX_EPS) {
                double rhs = tableau[i * width + width - 1];
                double ratio = rhs / coeff;
                if (ratio < best_ratio - SIMPLEX_EPS) {
                    best_ratio = ratio;
                    pivot_row = i;
                }
            }
        }

        if (pivot_row == height) {
            status = SIMPLEX_UNBOUNDED;
            goto cleanup;
        }

        // Pivot operation
        double pivot = tableau[pivot_row * width + pivot_col];
        if (fabs(pivot) < SIMPLEX_EPS) {
            status = SIMPLEX_INVALID_INPUT;
            goto cleanup;
        }

        // Normalize pivot row
        for (size_t j = 0; j < width; ++j) {
            tableau[pivot_row * width + j] /= pivot;
        }

        // Eliminate pivot column from other rows
        for (size_t i = 0; i < height; ++i) {
            if (i == pivot_row) {
                continue;
            }
            double factor = tableau[i * width + pivot_col];
            if (fabs(factor) <= SIMPLEX_EPS) {
                continue;
            }
            for (size_t j = 0; j < width; ++j) {
                tableau[i * width + j] -= factor * tableau[pivot_row * width + j];
            }
        }

        basis[pivot_row] = pivot_col;
    }

    memset(out_solution, 0, num_variables * sizeof(double));
    for (size_t i = 0; i < num_constraints; ++i) {
        size_t var_index = basis[i];
        if (var_index < num_variables) {
            out_solution[var_index] = tableau[i * width + width - 1];
        }
    }
    *out_optimal = tableau[obj_row + width - 1];

cleanup:
    free(tableau);
    free(basis);
    return status;
}

