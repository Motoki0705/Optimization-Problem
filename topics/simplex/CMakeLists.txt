add_library(simplex STATIC
    src/simplex.c
)

target_include_directories(simplex
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(simplex_cli src/simplex_cli.c)
target_link_libraries(simplex_cli PRIVATE simplex m)
