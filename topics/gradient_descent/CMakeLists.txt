add_library(gd STATIC
    src/gradient_descent.cpp
)

target_compile_features(gd PUBLIC cxx_std_17)

target_include_directories(gd
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(gd1d examples/gd1d.cpp)
target_link_libraries(gd1d PRIVATE gd)

target_compile_features(gd1d PRIVATE cxx_std_17)

add_executable(gd2d examples/gd2d.cpp)
target_link_libraries(gd2d PRIVATE gd)

target_compile_features(gd2d PRIVATE cxx_std_17)
