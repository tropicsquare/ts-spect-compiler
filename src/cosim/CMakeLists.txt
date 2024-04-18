add_library(spect_iss_dpi SHARED
    spect_iss_dpi.cpp
)

add_library(spect_iss_lib SHARED
    spect_iss_lib.cpp
)

target_link_libraries(spect_iss_dpi
    SPECT
    COMMON
    XKCP
)

target_link_libraries(spect_iss_lib
    SPECT
    COMMON
    XKCP
)

target_include_directories(spect_iss_dpi PUBLIC $ENV{VCS_HOME}/include)

target_compile_definitions(spect_iss_lib PUBLIC
                             TOOL_VERSION_TAG=${TAG_STR}
                             TOOL_VERSION_HASH=${HASH_STR})

set_source_files_properties(spect_iss_dpi.cpp PROPERTIES COMPILE_FLAGS -Wno-write-strings)
