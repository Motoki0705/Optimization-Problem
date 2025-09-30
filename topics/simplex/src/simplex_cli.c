#include "simplex/simplex.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_BUFFER 4096

static void trim_newline(char *line) {
    size_t len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[--len] = '\0';
    }
}

static int read_effective_line(FILE *fp, char *buffer, size_t size) {
    while (fgets(buffer, (int)size, fp)) {
        trim_newline(buffer);
        char *p = buffer;
        while (*p && isspace((unsigned char)*p)) {
            ++p;
        }
        if (*p == '\0' || *p == '#') {
            continue;
        }
        if (p != buffer) {
            memmove(buffer, p, strlen(p) + 1);
        }
        return 1;
    }
    return 0;
}

static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s --input <path>\n", prog);
    fprintf(stderr, "Input format:\n");
    fprintf(stderr, "  Line 1: <num_constraints> <num_variables>\n");
    fprintf(stderr, "  Line 2: objective coefficients (length = num_variables)\n");
    fprintf(stderr, "  Next lines: each constraint has num_variables coefficients followed by RHS\n");
    fprintf(stderr, "  Lines beginning with # or blank lines are ignored.\n");
}

int main(int argc, char **argv) {
    const char *input_path = NULL;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--input") == 0 && i + 1 < argc) {
            input_path = argv[++i];
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            usage(argv[0]);
            return 1;
        }
    }

    if (!input_path) {
        usage(argv[0]);
        return 1;
    }

    FILE *fp = fopen(input_path, "r");
    if (!fp) {
        fprintf(stderr, "Failed to open %s: %s\n", input_path, strerror(errno));
        return 2;
    }

    char line[LINE_BUFFER];
    if (!read_effective_line(fp, line, sizeof(line))) {
        fprintf(stderr, "Input is empty\n");
        fclose(fp);
        return 3;
    }

    char *endptr = NULL;
    errno = 0;
    unsigned long ul_constraints = strtoul(line, &endptr, 10);
    if (errno || endptr == line) {
        fprintf(stderr, "Failed to parse number of constraints\n");
        fclose(fp);
        return 3;
    }
    while (*endptr && isspace((unsigned char)*endptr)) {
        ++endptr;
    }
    errno = 0;
    unsigned long ul_variables = strtoul(endptr, &endptr, 10);
    if (errno || ul_variables == 0) {
        fprintf(stderr, "Failed to parse number of variables\n");
        fclose(fp);
        return 3;
    }

    size_t num_constraints = (size_t)ul_constraints;
    size_t num_variables = (size_t)ul_variables;

    if (!read_effective_line(fp, line, sizeof(line))) {
        fprintf(stderr, "Missing objective coefficients\n");
        fclose(fp);
        return 3;
    }

    double *objective = (double *)calloc(num_variables, sizeof(double));
    double *constraints = (double *)calloc(num_constraints * num_variables, sizeof(double));
    double *rhs = (double *)calloc(num_constraints, sizeof(double));
    double *solution = (double *)calloc(num_variables, sizeof(double));
    if (!objective || !constraints || !rhs || !solution) {
        fprintf(stderr, "Failed to allocate memory\n");
        fclose(fp);
        free(objective);
        free(constraints);
        free(rhs);
        free(solution);
        return 3;
    }

    char *cursor = line;
    for (size_t j = 0; j < num_variables; ++j) {
        errno = 0;
        objective[j] = strtod(cursor, &endptr);
        if (errno || cursor == endptr) {
            fprintf(stderr, "Failed to parse objective coefficient %zu\n", j);
            goto cleanup;
        }
        cursor = endptr;
    }

    for (size_t i = 0; i < num_constraints; ++i) {
        if (!read_effective_line(fp, line, sizeof(line))) {
            fprintf(stderr, "Missing constraint row %zu\n", i);
            goto cleanup;
        }
        cursor = line;
        for (size_t j = 0; j < num_variables; ++j) {
            errno = 0;
            constraints[i * num_variables + j] = strtod(cursor, &endptr);
            if (errno || cursor == endptr) {
                fprintf(stderr, "Failed to parse constraint (%zu, %zu)\n", i, j);
                goto cleanup;
            }
            cursor = endptr;
        }
        errno = 0;
        rhs[i] = strtod(cursor, &endptr);
        if (errno || cursor == endptr) {
            fprintf(stderr, "Failed to parse RHS for constraint %zu\n", i);
            goto cleanup;
        }
    }

    fclose(fp);
    fp = NULL;

    double optimal = 0.0;
    SimplexStatus status = simplex_solve(num_constraints, num_variables, constraints, rhs, objective, solution, &optimal);
    if (status != SIMPLEX_OK) {
        fprintf(stderr, "Simplex failed: %s\n", simplex_status_str(status));
        goto cleanup;
    }

    printf("Optimal value: %.10f\n", optimal);
    for (size_t j = 0; j < num_variables; ++j) {
        printf("x%zu = %.10f\n", j + 1, solution[j]);
    }

    free(objective);
    free(constraints);
    free(rhs);
    free(solution);
    return 0;

cleanup:
    if (fp) {
        fclose(fp);
    }
    free(objective);
    free(constraints);
    free(rhs);
    free(solution);
    return 3;
}

