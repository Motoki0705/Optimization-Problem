#include "gd/gradient_descent.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <memory>
#include <vector>

namespace {

struct Args {
    double a11{0.0};
    double a22{0.0};
    double a12{0.0};
    double b1{0.0};
    double b2{0.0};
    double c0{0.0};
    double alpha{0.1};
    double eps{1e-3};
    std::size_t maxIters{200};
    double x1{0.0};
    double x2{0.0};
    std::string csvPath{"outputs/out2d.csv"};
};

void usage(const char *prog) {
    std::cerr << "Usage: " << prog
              << " --a11 <v> --a22 <v> --a12 <v> --b1 <v> --b2 <v> --c0 <v>"
              << " --alpha <v> --eps <v> --max-iters <n> --x1 <v> --x2 <v> --csv <path>\n";
}

bool parseArgs(int argc, char **argv, Args &args) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--a11") == 0 && i + 1 < argc) {
            args.a11 = std::atof(argv[++i]);
        } else if (std::strcmp(argv[i], "--a22") == 0 && i + 1 < argc) {
            args.a22 = std::atof(argv[++i]);
        } else if (std::strcmp(argv[i], "--a12") == 0 && i + 1 < argc) {
            args.a12 = std::atof(argv[++i]);
        } else if (std::strcmp(argv[i], "--b1") == 0 && i + 1 < argc) {
            args.b1 = std::atof(argv[++i]);
        } else if (std::strcmp(argv[i], "--b2") == 0 && i + 1 < argc) {
            args.b2 = std::atof(argv[++i]);
        } else if (std::strcmp(argv[i], "--c0") == 0 && i + 1 < argc) {
            args.c0 = std::atof(argv[++i]);
        } else if (std::strcmp(argv[i], "--alpha") == 0 && i + 1 < argc) {
            args.alpha = std::atof(argv[++i]);
        } else if (std::strcmp(argv[i], "--eps") == 0 && i + 1 < argc) {
            args.eps = std::atof(argv[++i]);
        } else if (std::strcmp(argv[i], "--max-iters") == 0 && i + 1 < argc) {
            args.maxIters = static_cast<std::size_t>(std::strtoull(argv[++i], nullptr, 10));
        } else if (std::strcmp(argv[i], "--x1") == 0 && i + 1 < argc) {
            args.x1 = std::atof(argv[++i]);
        } else if (std::strcmp(argv[i], "--x2") == 0 && i + 1 < argc) {
            args.x2 = std::atof(argv[++i]);
        } else if (std::strcmp(argv[i], "--csv") == 0 && i + 1 < argc) {
            args.csvPath = argv[++i];
        } else {
            usage(argv[0]);
            return false;
        }
    }
    return true;
}

class QuadraticObjective final : public gd::Objective {
public:
    QuadraticObjective(double a11, double a22, double a12, double b1, double b2, double c0)
        : Objective(2), a11_(a11), a22_(a22), a12_(a12), b1_(b1), b2_(b2), c0_(c0) {}

    double value(const gd::Vector &x) const override {
        const double x1 = x[0];
        const double x2 = x[1];
        return a11_ * x1 * x1 + a22_ * x2 * x2 + a12_ * x1 * x2 + b1_ * x1 + b2_ * x2 + c0_;
    }

    bool hasAnalyticGradient() const noexcept override { return true; }

    gd::Vector analyticGradient(const gd::Vector &x) const override {
        const double x1 = x[0];
        const double x2 = x[1];
        return {2.0 * a11_ * x1 + a12_ * x2 + b1_, 2.0 * a22_ * x2 + a12_ * x1 + b2_};
    }

private:
    double a11_;
    double a22_;
    double a12_;
    double b1_;
    double b2_;
    double c0_;
};

} // namespace

int main(int argc, char **argv) {
    Args args;
    if (!parseArgs(argc, argv, args)) {
        return EXIT_FAILURE;
    }

    try {
        QuadraticObjective objective(args.a11, args.a22, args.a12, args.b1, args.b2, args.c0);
        gd::Vector x{args.x1, args.x2};

        gd::OptimConfig config;
        config.learningRate = args.alpha;
        config.tolerance = args.eps;
        config.maxIterations = args.maxIters;

        gd::Trainer trainer;
        auto csvLogger = std::make_shared<gd::CsvLogger>(args.csvPath);
        std::vector<std::shared_ptr<gd::Callback>> callbacks{csvLogger};

        const gd::TrainStats stats = trainer.minimize(objective, x, config, callbacks);
        std::cout << "Final value: " << stats.finalValue
                  << " after " << stats.iterations << " iterations" << std::endl;
        std::cout << "Minimizer x = (" << x[0] << ", " << x[1] << ")" << std::endl;

    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

