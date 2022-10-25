
cmake_minimum_required ( VERSION 3.18.2 )

project( SPECT_COMPILER C CXX)

SET(CMAKE_CXX_FLAGS "-O0")

include_directories(
    "src/common"
    "src/spect_lib"
    "src/cosim"
)

add_compile_options(-Wall -Wextra -fPIC -fdump-tree-original)

enable_testing()

add_subdirectory(src)
add_subdirectory(test)