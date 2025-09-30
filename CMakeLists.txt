cmake_minimum_required(VERSION 3.16)
project(optim LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_subdirectory(topics/gradient_descent)
add_subdirectory(topics/simplex)
