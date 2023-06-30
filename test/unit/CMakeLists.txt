

set (CC spect_compiler)
set (CARGS --first-address=0x8000 --isa-version=1)

macro(ADD_UNIT_TEST TEST_NAME)
    add_test(NAME ${TEST_NAME}_COMPILE COMMAND ${CC} ${CARGS} --hex-file=${CMAKE_CURRENT_SOURCE_DIR}/build/${TEST_NAME}.hex
                                                      ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}.s)

    # Compare with "gold" -> Current version of assembled files
    add_test(NAME ${TEST_NAME}_CHECK COMMAND diff ${CMAKE_CURRENT_SOURCE_DIR}/build/${TEST_NAME}.hex ${CMAKE_CURRENT_SOURCE_DIR}/golden/${TEST_NAME}.hex)
endmacro()

ADD_UNIT_TEST(type_r_test)
ADD_UNIT_TEST(type_i_test)
ADD_UNIT_TEST(type_j_test)
ADD_UNIT_TEST(type_m_test)

ADD_UNIT_TEST(eq_test)
ADD_UNIT_TEST(include_test)