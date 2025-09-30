#include "gd/gradient_descent.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace gd {

Objective::Objective(std::size_t dimension, double finiteDifferenceStep)
    : dimension_(dimension), finiteDifferenceStep_(finiteDifferenceStep) {
    if (dimension_ == 0) {
        throw std::invalid_argument("Objective dimension must be positive");
    }
    if (!(finiteDifferenceStep_ > 0.0)) {
        finiteDifferenceStep_ = 1e-6;
    }
}

void Objective::setFiniteDifferenceStep(double step) noexcept {
    if (step > 0.0) {
        finiteDifferenceStep_ = step;
    }
}

Vector Objective::analyticGradient(const Vector &x) const {
    (void)x;
    throw std::logic_error("Analytic gradient not implemented");
}

void Objective::ensureDimension(const Vector &x) const {
    if (x.size() != dimension_) {
        throw std::invalid_argument("Vector dimension mismatch");
    }
}

Vector Objective::gradient(const Vector &x) const {
    ensureDimension(x);
    if (hasAnalyticGradient()) {
        return analyticGradient(x);
    }

    Vector grad(dimension_, 0.0);
    Vector xPerturbed = x;
    const double step = finiteDifferenceStep_;
    for (std::size_t i = 0; i < dimension_; ++i) {
        const double original = xPerturbed[i];
        xPerturbed[i] = original + step;
        const double forward = value(xPerturbed);
        xPerturbed[i] = original - step;
        const double backward = value(xPerturbed);
        grad[i] = (forward - backward) / (2.0 * step);
        xPerturbed[i] = original;
    }
    return grad;
}

void OptimConfig::applyDefaults() {
    if (!(learningRate > 0.0)) {
        learningRate = 0.1;
    }
    if (!(tolerance > 0.0)) {
        tolerance = 1e-2;
    }
    if (maxIterations == 0) {
        maxIterations = 100;
    }
    if (!(numericGradientStep > 0.0)) {
        numericGradientStep = 1e-6;
    }
}

void GradientDescentOptimizer::step(const OptimConfig &config, Vector &x, const Vector &gradient) const {
    for (std::size_t i = 0; i < x.size(); ++i) {
        x[i] -= config.learningRate * gradient[i];
    }
}

CsvLogger::CsvLogger(std::string path)
    : path_(std::move(path)) {}

CsvLogger::~CsvLogger() = default;

void CsvLogger::ensureStream() {
    if (stream_) {
        return;
    }
    ownedStream_ = std::make_unique<std::ofstream>(path_, std::ios::out | std::ios::trunc);
    std::ofstream *file = dynamic_cast<std::ofstream *>(ownedStream_.get());
    if (!file || !*file) {
        throw std::runtime_error("Failed to open CSV file: " + path_);
    }
    stream_ = ownedStream_.get();
}

void CsvLogger::onIteration(const TrainerState &state) {
    ensureStream();
    if (!wroteHeader_) {
        (*stream_) << "iter,value,grad_norm_inf,lr";
        for (std::size_t i = 0; i < state.parameters.size(); ++i) {
            (*stream_) << ",x" << i + 1;
        }
        (*stream_) << '\n';
        wroteHeader_ = true;
    }

    (*stream_) << state.iteration << ','
               << std::setprecision(17) << state.value << ','
               << state.gradNormInf << ','
               << state.config.learningRate;
    for (double xi : state.parameters) {
        (*stream_) << ',' << xi;
    }
    (*stream_) << '\n';
    stream_->flush();
}

ConsoleLogger::ConsoleLogger(std::ostream &out)
    : stream_(&out) {}

std::ostream &ConsoleLogger::defaultStream() {
    return std::cout;
}

void ConsoleLogger::onIteration(const TrainerState &state) {
    (*stream_) << "iter=" << state.iteration
               << " value=" << std::setprecision(6) << state.value
               << " |grad|_inf=" << state.gradNormInf
               << " lr=" << state.config.learningRate
               << '\n';
}

LearningRateDecay::LearningRateDecay(double factor, std::size_t period, double minLearningRate)
    : factor_(factor), period_(period), minLearningRate_(minLearningRate) {}

void LearningRateDecay::onIteration(const TrainerState &state) {
    if (period_ == 0 || state.iteration == 0) {
        return;
    }
    if (state.iteration % period_ == 0) {
        state.config.learningRate = std::max(state.config.learningRate * factor_, minLearningRate_);
    }
}

EarlyStop::EarlyStop(double targetValue)
    : targetValue_(targetValue) {}

void EarlyStop::onIteration(const TrainerState &state) {
    if (state.value <= targetValue_) {
        state.stop = true;
    }
}

TrainStats Trainer::minimize(Objective &objective,
                             Vector &x,
                             OptimConfig &config,
                             const std::vector<std::shared_ptr<Callback>> &callbacks) const {
    objective.ensureDimension(x);
    config.applyDefaults();
    objective.setFiniteDifferenceStep(config.numericGradientStep);

    GradientDescentOptimizer optimizer;
    TrainStats stats;
    Vector grad(objective.dimension(), 0.0);
    bool stop = false;

    for (std::size_t iter = 0; iter < config.maxIterations; ++iter) {
        const double value = objective.value(x);
        grad = objective.gradient(x);
        const double gradNorm = infNorm(grad);

        stats.iterations = iter + 1;
        stats.finalValue = value;
        stats.finalGradNorm = gradNorm;

        TrainerState state{iter, value, gradNorm, x, grad, objective, config, stop};
        for (const auto &cb : callbacks) {
            if (cb) {
                cb->onIteration(state);
            }
        }

        if (stop) {
            stats.stoppedEarly = true;
            break;
        }

        if (gradNorm < config.tolerance) {
            stats.converged = true;
            break;
        }

        optimizer.step(config, x, grad);
    }

    return stats;
}

double Trainer::infNorm(const Vector &values) {
    double norm = 0.0;
    for (double v : values) {
        norm = std::max(norm, std::fabs(v));
    }
    return norm;
}

} // namespace gd
