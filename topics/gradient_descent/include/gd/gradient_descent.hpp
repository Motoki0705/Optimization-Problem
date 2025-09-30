#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace gd {

using Vector = std::vector<double>;

// Forward decl
class Objective;
struct OptimConfig;
struct TrainerState;

// ------------------------- Objective -------------------------
class Objective {
public:
    explicit Objective(std::size_t dimension, double finiteDifferenceStep = 1e-6);
    virtual ~Objective() = default;

    std::size_t dimension() const noexcept { return dimension_; }

    // Compute objective value (must be implemented by concrete objectives)
    virtual double value(const Vector& x) const = 0;

    // Optional: analytic gradient; default throws
    virtual Vector analyticGradient(const Vector& x) const;

    // Whether analyticGradient() is implemented
    virtual bool hasAnalyticGradient() const noexcept { return false; }

    // Public gradient: uses analytic version if available, otherwise central differences
    Vector gradient(const Vector& x) const;

    // Finite-difference step control
    void setFiniteDifferenceStep(double step) noexcept;

protected:
    // Internal dimension guard (intentionally protected)
    void ensureDimension(const Vector& x) const;

private:
    std::size_t dimension_;
    double finiteDifferenceStep_;
};

// ------------------------- Optimizer Config -------------------------
struct OptimConfig {
    double learningRate = 0.1;
    double tolerance = 1e-2;
    std::size_t maxIterations = 100;
    double numericGradientStep = 1e-6;

    void applyDefaults();
};

// ------------------------- Callbacks API -------------------------
struct TrainerState {
    std::size_t iteration;
    double value;
    double gradNormInf;
    Vector& parameters;
    Vector& gradient;
    Objective& objective;
    OptimConfig& config;
    bool& stop;
};

struct Callback {
    virtual ~Callback() = default;
    virtual void onIteration(TrainerState& state) = 0; // non-const so callbacks may mutate state
};

// Logs to CSV file
class CsvLogger final : public Callback {
public:
    explicit CsvLogger(std::string path);
    ~CsvLogger() override;

    void onIteration(TrainerState& state) override;

private:
    void ensureStream();

    std::string path_;
    std::unique_ptr<std::ostream> ownedStream_;
    std::ostream* stream_ = nullptr;
    bool wroteHeader_ = false;
};

// Logs to provided ostream (default: std::cout via defaultStream())
class ConsoleLogger final : public Callback {
public:
    explicit ConsoleLogger(std::ostream& out = defaultStream());
    void onIteration(TrainerState& state) override;

    static std::ostream& defaultStream();

private:
    std::ostream* stream_;
};

// Decays LR every `period` steps by `factor`, bounded below by `minLearningRate`
class LearningRateDecay final : public Callback {
public:
    LearningRateDecay(double factor = 0.5, std::size_t period = 10, double minLearningRate = 1e-6);
    void onIteration(TrainerState& state) override;

private:
    double factor_;
    std::size_t period_;
    double minLearningRate_;
};

// Early stopping when value <= targetValue
class EarlyStop final : public Callback {
public:
    explicit EarlyStop(double targetValue);
    void onIteration(TrainerState& state) override;

private:
    double targetValue_;
};

// ------------------------- Optimizer & Trainer -------------------------
class GradientDescentOptimizer {
public:
    void step(const OptimConfig& config, Vector& x, const Vector& gradient) const;
};

struct TrainStats {
    std::size_t iterations = 0;
    double finalValue = 0.0;
    double finalGradNorm = 0.0;
    bool converged = false;
    bool stoppedEarly = false;
};

class Trainer {
public:
    TrainStats minimize(Objective& objective,
                        Vector& x,
                        OptimConfig& config,
                        const std::vector<std::shared_ptr<Callback>>& callbacks) const;

    static double infNorm(const Vector& values);
};

} // namespace gd

