#include "gd/gradient_descent.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <memory>
#include <vector>

namespace {

struct Args {
    double a3{0.0};
    double a2{0.0};
    double a1{0.0};
    double a0{0.0};
    double alpha{0.1};
    double eps{1e-3};
    std::size_t maxIters{100};
    double x0{0.0};
    std::string csvPath{"outputs/out.csv"};
};

void usage(const char *prog) {
    std::cerr << "Usage: " << prog
              << " --a3 <v> --a2 <v> --a1 <v> --a0 <v>"
              << " --alpha <v> --eps <v> --max-iters <n> --x0 <v> --csv <path>\n";
}

bool parseArgs(int argc, char **argv, Args &args) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--a3") == 0 && i + 1 < argc) {
            args.a3 = std::atof(argv[++i]);
        } else if (std::strcmp(argv[i], "--a2") == 0 && i + 1 < argc) {
            args.a2 = std::atof(argv[++i]);
        } else if (std::strcmp(argv[i], "--a1") == 0 && i + 1 < argc) {
            args.a1 = std::atof(argv[++i]);
        } else if (std::strcmp(argv[i], "--a0") == 0 && i + 1 < argc) {
            args.a0 = std::atof(argv[++i]);
        } else if (std::strcmp(argv[i], "--alpha") == 0 && i + 1 < argc) {
            args.alpha = std::atof(argv[++i]);
        } else if (std::strcmp(argv[i], "--eps") == 0 && i + 1 < argc) {
            args.eps = std::atof(argv[++i]);
        } else if (std::strcmp(argv[i], "--max-iters") == 0 && i + 1 < argc) {
            args.maxIters = static_cast<std::size_t>(std::strtoull(argv[++i], nullptr, 10));
        } else if (std::strcmp(argv[i], "--x0") == 0 && i + 1 < argc) {
            args.x0 = std::atof(argv[++i]);
        } else if (std::strcmp(argv[i], "--csv") == 0 && i + 1 < argc) {
            args.csvPath = argv[++i];
        } else {
            usage(argv[0]);
            return false;
        }
    }
    return true;
}

class CubicObjective final : public gd::Objective {
public:
    CubicObjective(double a3, double a2, double a1, double a0)
        : Objective(1), a3_(a3), a2_(a2), a1_(a1), a0_(a0) {}

    double value(const gd::Vector &x) const override {
        const double v = x[0];
        return ((a3_ * v + a2_) * v + a1_) * v + a0_;
    }

    bool hasAnalyticGradient() const noexcept override { return true; }

    gd::Vector analyticGradient(const gd::Vector &x) const override {
        const double v = x[0];
        return {3.0 * a3_ * v * v + 2.0 * a2_ * v + a1_};
    }

private:
    double a3_;
    double a2_;
    double a1_;
    double a0_;
};

} // namespace

int main(int argc, char **argv) {
    Args args;
    if (!parseArgs(argc, argv, args)) {
        return EXIT_FAILURE;
    }

    try {
        CubicObjective objective(args.a3, args.a2, args.a1, args.a0);
        gd::Vector x{args.x0};

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
        std::cout << "Minimizer x = " << x[0] << std::endl;

    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

