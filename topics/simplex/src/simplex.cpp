#include "simplex/simplex.hpp"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

namespace simplex {
namespace {
constexpr double kEps = 1e-9;

class Tableau {
public:
    Tableau(std::size_t height, std::size_t width)
        : width_(width), height_(height), data_(height * width, 0.0) {}

    double &operator()(std::size_t row, std::size_t col) {
        return data_[row * width_ + col];
    }

    double operator()(std::size_t row, std::size_t col) const {
        return data_[row * width_ + col];
    }

    std::size_t width() const noexcept { return width_; }
    std::size_t height() const noexcept { return height_; }

    double *rowPtr(std::size_t row) { return data_.data() + row * width_; }
    const double *rowPtr(std::size_t row) const { return data_.data() + row * width_; }

private:
    std::size_t width_;
    std::size_t height_;
    std::vector<double> data_;
};

} // namespace

std::string statusToString(Status status) {
    switch (status) {
        case Status::Optimal:
            return "optimal";
        case Status::Unbounded:
            return "unbounded";
        case Status::Infeasible:
            return "infeasible";
        case Status::InvalidInput:
        default:
            return "invalid_input";
    }
}

Solution SimplexSolver::solve(const Problem &problem) const {
    Solution solution;
    solution.status = Status::InvalidInput;

    const std::size_t m = problem.numConstraints;
    const std::size_t n = problem.numVariables;

    if (m == 0 || n == 0) {
        return solution;
    }
    if (problem.A.size() != m * n || problem.b.size() != m || problem.c.size() != n) {
        return solution;
    }

    for (double rhs : problem.b) {
        if (rhs < -kEps) {
            solution.status = Status::Infeasible;
            return solution;
        }
    }

    const std::size_t width = n + m + 1; // variables + slacks + RHS
    const std::size_t height = m + 1;    // constraints + objective

    Tableau tableau(height, width);
    std::vector<std::size_t> basis(m, 0);

    // Populate constraints
    for (std::size_t i = 0; i < m; ++i) {
        const double *rowCoeffs = problem.A.data() + i * n;
        std::copy(rowCoeffs, rowCoeffs + n, tableau.rowPtr(i));
        tableau(i, n + i) = 1.0;
        tableau(i, width - 1) = problem.b[i];
        basis[i] = n + i;
    }

    // Objective row (maximization)
    const std::size_t objectiveRow = m;
    for (std::size_t j = 0; j < n; ++j) {
        tableau(objectiveRow, j) = -problem.c[j];
    }

    Status status = Status::Optimal;

    while (true) {
        double mostNegative = 0.0;
        std::size_t pivotCol = width; // invalid sentinel
        for (std::size_t j = 0; j < width - 1; ++j) {
            const double coeff = tableau(objectiveRow, j);
            if (coeff < mostNegative - kEps) {
                mostNegative = coeff;
                pivotCol = j;
            }
        }

        if (pivotCol == width) {
            break; // optimal reached
        }

        double bestRatio = std::numeric_limits<double>::infinity();
        std::size_t pivotRow = height; // invalid sentinel
        for (std::size_t i = 0; i < m; ++i) {
            const double coeff = tableau(i, pivotCol);
            if (coeff > kEps) {
                const double rhs = tableau(i, width - 1);
                const double ratio = rhs / coeff;
                if (ratio < bestRatio - kEps) {
                    bestRatio = ratio;
                    pivotRow = i;
                }
            }
        }

        if (pivotRow == height) {
            status = Status::Unbounded;
            break;
        }

        const double pivot = tableau(pivotRow, pivotCol);
        if (std::fabs(pivot) < kEps) {
            status = Status::InvalidInput;
            break;
        }

        const double invPivot = 1.0 / pivot;
        for (std::size_t j = 0; j < width; ++j) {
            tableau(pivotRow, j) *= invPivot;
        }

        for (std::size_t i = 0; i < height; ++i) {
            if (i == pivotRow) {
                continue;
            }
            const double factor = tableau(i, pivotCol);
            if (std::fabs(factor) <= kEps) {
                continue;
            }
            for (std::size_t j = 0; j < width; ++j) {
                tableau(i, j) -= factor * tableau(pivotRow, j);
            }
        }

        basis[pivotRow] = pivotCol;
    }

    solution.status = status;
    if (status != Status::Optimal) {
        return solution;
    }

    solution.variables.assign(n, 0.0);
    for (std::size_t i = 0; i < m; ++i) {
        const std::size_t basicVar = basis[i];
        if (basicVar < n) {
            solution.variables[basicVar] = tableau(i, width - 1);
        }
    }
    solution.objective = tableau(objectiveRow, width - 1);
    return solution;
}

} // namespace simplex
