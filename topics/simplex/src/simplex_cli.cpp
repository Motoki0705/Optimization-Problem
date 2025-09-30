#include "simplex/simplex.hpp"

#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

std::string trim(const std::string &line) {
    std::size_t start = 0;
    while (start < line.size() && std::isspace(static_cast<unsigned char>(line[start]))) {
        ++start;
    }
    std::size_t end = line.size();
    while (end > start && std::isspace(static_cast<unsigned char>(line[end - 1]))) {
        --end;
    }
    return line.substr(start, end - start);
}

bool readEffectiveLine(std::istream &in, std::string &line) {
    while (std::getline(in, line)) {
        const std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed.front() == '#') {
            continue;
        }
        line = trimmed;
        return true;
    }
    return false;
}

void usage(const char *prog) {
    std::cerr << "Usage: " << prog << " --input <path>\n";
    std::cerr << "File format:\n";
    std::cerr << "  <num_constraints> <num_variables>\n";
    std::cerr << "  <objective coefficients...>\n";
    std::cerr << "  constraint rows: <coefficients...> <rhs>\n";
    std::cerr << "Lines starting with # are ignored." << std::endl;
}

simplex::Problem parseProblem(const std::string &path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open input file: " + path);
    }

    simplex::Problem problem;
    std::string line;
    if (!readEffectiveLine(file, line)) {
        throw std::runtime_error("Input is empty");
    }

    {
        std::istringstream iss(line);
        if (!(iss >> problem.numConstraints >> problem.numVariables)) {
            throw std::runtime_error("Failed to parse problem dimensions");
        }
    }

    if (problem.numConstraints == 0 || problem.numVariables == 0) {
        throw std::runtime_error("Problem dimensions must be positive");
    }

    if (!readEffectiveLine(file, line)) {
        throw std::runtime_error("Missing objective coefficients");
    }

    {
        std::istringstream iss(line);
        problem.c.resize(problem.numVariables);
        for (std::size_t j = 0; j < problem.numVariables; ++j) {
            if (!(iss >> problem.c[j])) {
                throw std::runtime_error("Failed to parse objective coefficient " + std::to_string(j));
            }
        }
    }

    problem.A.resize(problem.numConstraints * problem.numVariables);
    problem.b.resize(problem.numConstraints);

    for (std::size_t i = 0; i < problem.numConstraints; ++i) {
        if (!readEffectiveLine(file, line)) {
            throw std::runtime_error("Missing constraint row " + std::to_string(i));
        }
        std::istringstream iss(line);
        for (std::size_t j = 0; j < problem.numVariables; ++j) {
            if (!(iss >> problem.A[i * problem.numVariables + j])) {
                throw std::runtime_error("Failed to parse constraint coefficient (" +
                                         std::to_string(i) + ", " + std::to_string(j) + ")");
            }
        }
        if (!(iss >> problem.b[i])) {
            throw std::runtime_error("Failed to parse constraint RHS " + std::to_string(i));
        }
    }

    return problem;
}

} // namespace

int main(int argc, char **argv) {
    std::string inputPath;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if ((arg == "--help") || (arg == "-h")) {
            usage(argv[0]);
            return EXIT_SUCCESS;
        } else if (arg == "--input" && i + 1 < argc) {
            inputPath = argv[++i];
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (inputPath.empty()) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    try {
        const simplex::Problem problem = parseProblem(inputPath);
        const simplex::SimplexSolver solver;
        const simplex::Solution result = solver.solve(problem);

        if (result.status != simplex::Status::Optimal) {
            std::cerr << "Simplex failed: " << simplex::statusToString(result.status) << std::endl;
            return EXIT_FAILURE;
        }

        std::cout << "Optimal value: " << result.objective << '\n';
        for (std::size_t i = 0; i < result.variables.size(); ++i) {
            std::cout << "x" << (i + 1) << " = " << result.variables[i] << '\n';
        }

    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
