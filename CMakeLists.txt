
cmake_minimum_required ( VERSION 3.18.2 )

project( SPECT_COMPILER C CXX)

set (CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS "-O0")

include_directories(
    "src/common"
    "src/spect_lib"
    "src/cosim"
    "$ENV{TS_REPO_ROOT}/modules/cli/include/cli"
)

add_compile_options(-Wall -Wextra -Wshadow -fPIC -fdump-tree-original)
add_link_options(-pthread)

enable_testing()

add_subdirectory(src)
add_subdirectory(test)