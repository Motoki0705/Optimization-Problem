cmake_minimum_required(VERSION 3.16)
project(optim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(topics/gradient_descent)
add_subdirectory(topics/simplex)
