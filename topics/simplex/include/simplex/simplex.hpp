#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace simplex {

struct Problem {
    std::size_t numConstraints{0};
    std::size_t numVariables{0};
    std::vector<double> A; // Row-major matrix numConstraints x numVariables
    std::vector<double> b;
    std::vector<double> c;
};

enum class Status {
    Optimal,
    Unbounded,
    Infeasible,
    InvalidInput
};

struct Solution {
    Status status{Status::InvalidInput};
    std::vector<double> variables;
    double objective{0.0};
};

class SimplexSolver {
public:
    Solution solve(const Problem &problem) const;
};

std::string statusToString(Status status);

} // namespace simplex

