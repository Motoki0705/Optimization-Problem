// examples/gd2d.c
// Build: gcc -std=c11 -O2 -Wall -Wextra examples/gd2d.c -o examples/gd2d -lm
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    double a11, a22, a12, b1, b2, c0; // quadratic coefficients
    double alpha;    // learning rate
    double eps;      // stop when ||grad||2 <= eps
    int    max_iters;
    double x1_0, x2_0;
    const char* csv_path;
} Args;

static void usage(const char* prog){
    fprintf(stderr,
        "Usage: %s "
        "--a11 <v> --a22 <v> --a12 <v> --b1 <v> --b2 <v> --c0 <v> "
        "--alpha <v> --eps <v> --max-iters <n> --x1 <v> --x2 <v> --csv <path>\n", prog);
}

static int parse_args(int argc, char** argv, Args* A){
    // sensible defaults
    A->a11=0; A->a22=0; A->a12=0; A->b1=0; A->b2=0; A->c0=0;
    A->alpha=0.1; A->eps=1e-3; A->max_iters=200; A->x1_0=0.0; A->x2_0=0.0;
    A->csv_path = "outputs/out2d.csv";
    for(int i=1;i<argc;i++){
        if      (!strcmp(argv[i],"--a11") && i+1<argc) A->a11 = atof(argv[++i]);
        else if (!strcmp(argv[i],"--a22") && i+1<argc) A->a22 = atof(argv[++i]);
        else if (!strcmp(argv[i],"--a12") && i+1<argc) A->a12 = atof(argv[++i]);
        else if (!strcmp(argv[i],"--b1")  && i+1<argc) A->b1  = atof(argv[++i]);
        else if (!strcmp(argv[i],"--b2")  && i+1<argc) A->b2  = atof(argv[++i]);
        else if (!strcmp(argv[i],"--c0")  && i+1<argc) A->c0  = atof(argv[++i]);
        else if (!strcmp(argv[i],"--alpha") && i+1<argc) A->alpha = atof(argv[++i]);
        else if (!strcmp(argv[i],"--eps")    && i+1<argc) A->eps   = atof(argv[++i]);
        else if (!strcmp(argv[i],"--max-iters") && i+1<argc) A->max_iters = atoi(argv[++i]);
        else if (!strcmp(argv[i],"--x1") && i+1<argc) A->x1_0 = atof(argv[++i]);
        else if (!strcmp(argv[i],"--x2") && i+1<argc) A->x2_0 = atof(argv[++i]);
        else if (!strcmp(argv[i],"--csv") && i+1<argc) A->csv_path = argv[++i];
        else { usage(argv[0]); return 0; }
    }
    return 1;
}

static inline double f_val(const Args* A, double x1, double x2){
    return A->a11*x1*x1 + A->a22*x2*x2 + A->a12*x1*x2 + A->b1*x1 + A->b2*x2 + A->c0;
}
static inline void f_grad(const Args* A, double x1, double x2, double* g1, double* g2){
    *g1 = 2.0*A->a11*x1 + A->a12*x2 + A->b1;
    *g2 = 2.0*A->a22*x2 + A->a12*x1 + A->b2;
}

int main(int argc, char** argv){
    Args A;
    if(!parse_args(argc, argv, &A)) return 2;

    FILE* fp = fopen(A.csv_path, "w");
    if(!fp){ perror("fopen csv"); return 3; }
    fprintf(fp, "iter,value,grad_norm,lr,x1,x2,g1,g2\n");

    double x1=A.x1_0, x2=A.x2_0;
    for(int it=0; it<=A.max_iters; ++it){
        double g1, g2;
        f_grad(&A, x1, x2, &g1, &g2);
        double fx = f_val(&A, x1, x2);
        double gn = hypot(g1, g2); // L2 norm

        fprintf(fp, "%d,%.15g,%.15g,%.15g,%.15g,%.15g,%.15g,%.15g\n",
                it, fx, gn, A.alpha, x1, x2, g1, g2);

        if(gn <= A.eps) break;
        x1 -= A.alpha * g1;
        x2 -= A.alpha * g2;
        if(!isfinite(x1) || !isfinite(x2)) break; // 発散対策
    }
    fclose(fp);
    return 0;
}

