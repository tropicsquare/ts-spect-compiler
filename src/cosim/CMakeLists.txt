add_library(spect_iss_dpi SHARED
    spect_iss_dpi.cpp
)

target_link_libraries(spect_iss_dpi
    SPECT
    COMMON
    XKCP
)

target_include_directories(spect_iss_dpi PUBLIC $ENV{VCS_HOME}/include)

set_source_files_properties(spect_iss_dpi.cpp PROPERTIES COMPILE_FLAGS -Wno-write-strings)
