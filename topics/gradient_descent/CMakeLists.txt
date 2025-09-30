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

add_executable(gd1d examples/gd1d.c)
target_link_libraries(gd1d PRIVATE gd m)

add_executable(gd2d examples/gd2d.c)
target_link_libraries(gd2d PRIVATE gd m)
