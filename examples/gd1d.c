// gd1d.c : 1D gradient descent for a cubic (or lower) polynomial
// Build: gcc -std=c11 -O2 -Wall -Wextra gd1d.c -o gd1d -lm
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    double a3, a2, a1, a0;  // f(x) = a3 x^3 + a2 x^2 + a1 x + a0
    double alpha;           // learning rate
    double eps;             // tolerance on |grad|
    int    max_iters;       // max iterations
    double x0;              // initial point
    const char* csv_path;   // output csv
} GD1D_Args;

static void usage(const char* prog) {
    fprintf(stderr,
        "Usage: %s --a3 <v> --a2 <v> --a1 <v> --a0 <v> "
        "--alpha <v> --eps <v> --max-iters <n> --x0 <v> --csv <path>\n", prog);
}

static int parse_args(int argc, char** argv, GD1D_Args* A) {
    // defaults (can be overridden)
    A->a3 = 0.0; A->a2 = 0.0; A->a1 = 0.0; A->a0 = 0.0;
    A->alpha = 0.1; A->eps = 1e-3; A->max_iters = 100; A->x0 = 0.0; A->csv_path = "outputs/out.csv";

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--a3") && i+1 < argc) A->a3 = atof(argv[++i]);
        else if (!strcmp(argv[i], "--a2") && i+1 < argc) A->a2 = atof(argv[++i]);
        else if (!strcmp(argv[i], "--a1") && i+1 < argc) A->a1 = atof(argv[++i]);
        else if (!strcmp(argv[i], "--a0") && i+1 < argc) A->a0 = atof(argv[++i]);
        else if (!strcmp(argv[i], "--alpha") && i+1 < argc) A->alpha = atof(argv[++i]);
        else if (!strcmp(argv[i], "--eps") && i+1 < argc) A->eps = atof(argv[++i]);
        else if (!strcmp(argv[i], "--max-iters") && i+1 < argc) A->max_iters = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--x0") && i+1 < argc) A->x0 = atof(argv[++i]);
        else if (!strcmp(argv[i], "--csv") && i+1 < argc) A->csv_path = argv[++i];
        else {
            usage(argv[0]); return 0;
        }
    }
    return 1;
}

static inline double f_val(const GD1D_Args* A, double x) {
    return ((A->a3*x + A->a2)*x + A->a1)*x + A->a0; // Horner
}
static inline double f_grad(const GD1D_Args* A, double x) {
    // df/dx = 3a3 x^2 + 2a2 x + a1
    return 3.0*A->a3*x*x + 2.0*A->a2*x + A->a1;
}

int main(int argc, char** argv) {
    GD1D_Args A;
    if (!parse_args(argc, argv, &A)) return 2;

    FILE* fp = fopen(A.csv_path, "w");
    if (!fp) { perror("fopen csv"); return 3; }
    fprintf(fp, "iter,value,grad,lr,x\n");

    double x = A.x0;
    for (int it = 0; it <= A.max_iters; ++it) {
        double g = f_grad(&A, x);
        double fx = f_val(&A, x);
        fprintf(fp, "%d,%.15g,%.15g,%.15g,%.15g\n", it, fx, fabs(g), A.alpha, x);

        if (fabs(g) <= A.eps) {
            // converged by gradient norm
            break;
        }
        x = x - A.alpha * g;
    }
    fclose(fp);

    // 終了状態を標準出力にも軽く表示（任意）
    // printf("final x=%.10f, f=%.10f\n", x, f_val(&A,x));
    return 0;
}

