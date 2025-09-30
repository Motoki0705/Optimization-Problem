#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace gd {

using Vector = std::vector<double>;

class Objective {
public:
    explicit Objective(std::size_t dimension, double finiteDifferenceStep = 1e-6);
    virtual ~Objective() = default;

    std::size_t dimension() const noexcept { return dimension_; }
    double finiteDifferenceStep() const noexcept { return finiteDifferenceStep_; }
    void setFiniteDifferenceStep(double step) noexcept;  // step > 0 assumed

    virtual double value(const Vector &x) const = 0;
    virtual bool hasAnalyticGradient() const noexcept { return false; }
    virtual Vector analyticGradient(const Vector &x) const;

    Vector gradient(const Vector &x) const;

protected:
    void ensureDimension(const Vector &x) const;

private:
    std::size_t dimension_;
    double finiteDifferenceStep_;
};

struct OptimConfig {
    double learningRate{0.1};
    double tolerance{1e-2};
    std::size_t maxIterations{100};
    double numericGradientStep{1e-6};

    void applyDefaults();
};

class GradientDescentOptimizer {
public:
    void step(const OptimConfig &config, Vector &x, const Vector &gradient) const;
};

struct TrainStats {
    std::size_t iterations{0};
    double finalValue{0.0};
    double finalGradNorm{0.0};
    bool converged{false};
    bool stoppedEarly{false};
};

struct TrainerState {
    std::size_t iteration;
    double value;
    double gradNormInf;
    const Vector &parameters;
    const Vector &gradient;
    Objective &objective;
    OptimConfig &config;
    bool &stop;
};

class Callback {
public:
    virtual ~Callback() = default;
    virtual void onIteration(const TrainerState &state) = 0;
};

class CsvLogger : public Callback {
public:
    explicit CsvLogger(std::string path);
    ~CsvLogger() override;

    void onIteration(const TrainerState &state) override;

    const std::string &path() const noexcept { return path_; }

private:
    std::string path_;
    std::unique_ptr<std::ostream> ownedStream_;
    std::ostream *stream_{nullptr};
    bool wroteHeader_{false};

    void ensureStream();
};

class ConsoleLogger : public Callback {
public:
    explicit ConsoleLogger(std::ostream &out = defaultStream());

    void onIteration(const TrainerState &state) override;

private:
    std::ostream *stream_;
    static std::ostream &defaultStream();
};

class LearningRateDecay : public Callback {
public:
    LearningRateDecay(double factor, std::size_t period, double minLearningRate = 1e-6);
    void onIteration(const TrainerState &state) override;

private:
    double factor_;
    std::size_t period_;
    double minLearningRate_;
};

class EarlyStop : public Callback {
public:
    explicit EarlyStop(double targetValue);
    void onIteration(const TrainerState &state) override;

private:
    double targetValue_;
};

class Trainer {
public:
    TrainStats minimize(Objective &objective,
                        Vector &x,
                        OptimConfig &config,
                        const std::vector<std::shared_ptr<Callback>> &callbacks = {}) const;

private:
    static double infNorm(const Vector &values);
};

} // namespace gd
