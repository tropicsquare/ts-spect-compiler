add_library(spect_iss_dpi SHARED
    spect_iss_dpi.cpp
)

target_link_libraries(spect_iss_dpi
    SPECT
    COMMON
)