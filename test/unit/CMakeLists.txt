

set (CC spect_compiler)
set (CARGS --first-address=0x8000)

macro(ADD_UNIT_TEST TEST_NAME)
    add_test(NAME ${TEST_NAME} COMMAND ${CC} ${CARGS} --sim-hex=${CMAKE_CURRENT_SOURCE_DIR}/build/${TEST_NAME}.hex
                                                      ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}.s)
endmacro()

ADD_UNIT_TEST(type_r_test)
ADD_UNIT_TEST(type_i_test)
ADD_UNIT_TEST(type_j_test)
ADD_UNIT_TEST(type_m_test)

ADD_UNIT_TEST(eq_test)
ADD_UNIT_TEST(include_test)