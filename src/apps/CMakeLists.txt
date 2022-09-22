
add_executable(spect_compiler
    spect_compiler.cpp
)

target_link_libraries(spect_compiler
    SPECT
    COMMON
)

add_executable(spect_iss
    spect_iss.cpp
)

target_link_libraries(spect_iss
    SPECT
    COMMON
)