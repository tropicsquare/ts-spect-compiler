
macro(ADD_DPI_TEST TEST_NAME)
    add_executable(${TEST_NAME}
        ${TEST_NAME}.cpp
    )
    target_link_libraries(${TEST_NAME}
        spect_iss_dpi
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

    target_compile_definitions(${TEST_NAME} PUBLIC DPI_TEST_FW="${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}.s")
endmacro()


ADD_DPI_TEST(dpi_simple_test)
