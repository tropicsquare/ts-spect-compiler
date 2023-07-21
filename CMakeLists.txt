cmake_minimum_required ( VERSION 3.18.2 )

project( SPECT_COMPILER C CXX)

set (CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS "-O0")
#set(CMAKE_VERBOSE_MAKEFILE ON)

set(TS_SPECT_COMPILER_ROOT "${CMAKE_CURRENT_LIST_DIR}")

include_directories(
    "src/common"
    "src/spect_lib"
    "src/cosim"
    "modules/cli/include/cli"
    "modules/xkcp/bin/generic64/libXKCP.so.headers"
)

add_compile_options(-Wall -Wextra -Wshadow -fPIC -fdump-tree-original)
add_link_options(-pthread)

enable_testing()

add_subdirectory(src)
add_subdirectory(test)
