
add_executable(spect_compiler
    spect_compiler.cpp
)

target_link_libraries(spect_compiler
    SPECT
    COMMON
    XKCP
)

add_executable(spect_iss
    spect_iss.cpp
)

target_link_libraries(spect_iss
    SPECT
    COMMON
    XKCP
)

###################################################################################################
# Add SW versions
###################################################################################################
execute_process(COMMAND git tag --points-at HEAD OUTPUT_VARIABLE TAG_STR OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND git rev-parse --short HEAD OUTPUT_VARIABLE HASH_STR OUTPUT_STRIP_TRAILING_WHITESPACE)
set(TAG_STR "\"${TAG_STR}\"")
set(HASH_STR "\"${HASH_STR}\"")

message("${TAG_STR}")

target_compile_definitions(spect_compiler PUBLIC
                             TOOL_VERSION_TAG=${TAG_STR}
                             TOOL_VERSION_HASH=${HASH_STR})

target_compile_definitions(spect_iss PUBLIC
                             TOOL_VERSION_TAG=${TAG_STR}
                             TOOL_VERSION_HASH=${HASH_STR})


