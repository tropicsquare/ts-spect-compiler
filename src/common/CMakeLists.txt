
add_library (COMMON STATIC
    Sha512.cpp
)

add_custom_target(XKCP_BUILD ALL
    COMMENT "Generating libXKCP.a..."
    COMMAND $(MAKE) generic64/libXKCP.a
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/modules/xkcp
)

add_library(XKCP STATIC IMPORTED GLOBAL)
set_target_properties(XKCP PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/modules/xkcp/bin/generic64/libXKCP.a
    #INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/modules/xkcp/bin/generic64/libXKCP.a.headers
    #IMPORTED_NO_SONAME ON
)
