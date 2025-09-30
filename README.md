# Optim Playground

This repository collects optimization routines as independent "topics" that can evolve separately. Each topic provides its own headers, sources, examples, and shell helpers so that you can experiment with the idea in isolation and still share infrastructure such as the build system.

## Requirements

- C++17 capable compiler (tested with GCC/Clang)
- CMake 3.16 or newer
- POSIX shell environment for the helper scripts

If CMake is missing, install it with your package manager (for example `sudo apt install cmake`).

## Layout

```
topics/
  gradient_descent/   Core library, examples, and headers for the GD toolkit
  simplex/            Simplex method implementation and CLI
docs/                 Background notes (Japanese)
scripts/              Shell helpers to build and run each topic
Makefile, CMakeLists.txt
```

Each topic exports a static library and optional executables. New topics can follow the same pattern: create a folder under `topics/`, add a `CMakeLists.txt`, and expose helper scripts under `scripts/`.

## Building Everything

```
make          # configure + build all targets into ./build
make clean    # remove the build directory
```

The generated binaries live in `build/topics/<topic-name>/`.

## Topic: Gradient Descent

Gradient based minimisation utilities implemented with a small set of C++ classes (`gd::Objective`, `gd::Trainer`, callbacks, etc.). The helper script compiles the library and runs a demo problem, writing the optimisation trace as CSV.

```
scripts/run_gradient_descent.sh --example 1d   # cubic polynomial minimum (default)
scripts/run_gradient_descent.sh --example 2d   # quadratic surface minimum
```

The CSV files are placed under `topics/gradient_descent/examples/outputs/` for easy plotting.

## Topic: Simplex Method

Implements the primal simplex algorithm for linear programmes in standard form (maximize `c^T x` subject to `A x <= b`, `x >= 0`). A small CLI wraps the solver and reads a plain-text input format:

```
<num_constraints> <num_variables>
c1 c2 ... cn
row1_coeffs... rhs
row2_coeffs... rhs
...
```

Lines beginning with `#` or blank lines are ignored. An example lives at `topics/simplex/examples/sample.lp`.

Run the demo with:

```
scripts/run_simplex.sh
# or specify another input file
scripts/run_simplex.sh --input /path/to/problem.lp
```

The CLI prints the optimal objective value and the decision variables. For infeasible inputs (e.g. constraints with negative RHS) the solver reports the corresponding status code.

## Extending the Repository

1. Create `topics/<your-topic>/` with `include/`, `src/`, `examples/` and a `CMakeLists.txt`.
2. Export a library and, if useful, add CLI or demo executables.
3. Provide a shell helper in `scripts/` describing how to build/run the topic.
4. Document the new topic in this README.

Refer to `docs/ARCHITECTURE_JA.md` for an overview of the gradient descent module internals. Update or add new documents under `docs/` as topics evolve.
