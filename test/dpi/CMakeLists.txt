
macro(ADD_LIB_TEST TEST_NAME LIB_TO_LINK)
    add_executable(${TEST_NAME}
        ${TEST_NAME}.cpp
    )
    target_link_libraries(${TEST_NAME}
        ${LIB_TO_LINK}
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

    target_compile_definitions(${TEST_NAME} PUBLIC DPI_TEST_FW="${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}.s")
endmacro()


ADD_LIB_TEST(dpi_simple_test        spect_iss_dpi)
ADD_LIB_TEST(iss_lib_simple_test    spect_iss_lib)
