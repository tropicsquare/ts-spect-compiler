
add_subdirectory(common)
add_subdirectory(spect_lib)
add_subdirectory(apps)


###############################################################################
# Check availability of VCS (currently COSIM is only available for VCS)
###############################################################################

if(DEFINED ENV{VCS_HOME})
    message(STATUS "Detected VCS...")
    message(STATUS "Building DPI/VPI cosimulation library")
    add_subdirectory(cosim)
    add_definitions(-DUSING_VCS)
else()
    message(WARNING "VCS not detected, skipping build of cosimulation library...")
endif()
