
add_library (COMMON STATIC
    Sha512.cpp
)

add_custom_target(XKCP_BUILD ALL
    COMMENT "Generating libXKCP.so..."
    COMMAND $(MAKE) generic64/libXKCP.so
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/modules/xkcp
)

add_library(XKCP SHARED IMPORTED GLOBAL)
set_target_properties(XKCP PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/modules/xkcp/bin/generic64/libXKCP.so
    #INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/modules/xkcp/bin/generic64/libXKCP.so.headers
    IMPORTED_NO_SONAME ON
)
