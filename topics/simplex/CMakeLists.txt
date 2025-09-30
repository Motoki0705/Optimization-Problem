add_library(simplex STATIC
    src/simplex.cpp
)

target_compile_features(simplex PUBLIC cxx_std_17)

target_include_directories(simplex
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(simplex_cli src/simplex_cli.cpp)
target_link_libraries(simplex_cli PRIVATE simplex)

target_compile_features(simplex_cli PRIVATE cxx_std_17)
