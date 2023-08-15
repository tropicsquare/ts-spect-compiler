
add_subdirectory(unit)

if(DEFINED ENV{VCS_HOME})
    message(STATUS "Detected VCS...")
    message(STATUS "Building DPI library tests")
    add_subdirectory(dpi)
else()
    message(WARNING "VCS not detected, skipping build of DPI library tests...")
endif()

add_subdirectory(timing)
