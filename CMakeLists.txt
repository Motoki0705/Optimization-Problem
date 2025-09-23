cmake_minimum_required(VERSION 3.10)
project(gd_optim_framework C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(gd STATIC
    src/model.c
    src/loss.c
    src/optim.c
    src/trainer.c
    src/callbacks.c
    src/utils.c
)

target_include_directories(gd
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(gd_quadratic examples/gd_quadratic.c)
target_link_libraries(gd_quadratic gd m)

add_executable(gd_sincos examples/gd_sincos.c)
target_link_libraries(gd_sincos gd m)
