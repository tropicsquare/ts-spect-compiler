

set (CC spect_compiler)

macro(ADD_UNIT_TEST TEST_NAME ISA_VERSION FIRST_ADDRESS)
    add_test(NAME ${TEST_NAME}_COMPILE COMMAND ${CC} --isa-version=${ISA_VERSION} --hex-file=${CMAKE_CURRENT_SOURCE_DIR}/build/${TEST_NAME}.hex
                                                      --first-address=${FIRST_ADDRESS} ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}.s)

    # Compare with "gold" -> Current version of assembled files
    add_test(NAME ${TEST_NAME}_CHECK COMMAND diff ${CMAKE_CURRENT_SOURCE_DIR}/build/${TEST_NAME}.hex ${CMAKE_CURRENT_SOURCE_DIR}/golden/${TEST_NAME}.hex)
endmacro()

ADD_UNIT_TEST(type_r_test 1 0x8000)
ADD_UNIT_TEST(type_i_test 1 0x8000)
ADD_UNIT_TEST(type_j_test 1 0x8000)
ADD_UNIT_TEST(type_m_test 1 0x8100)

ADD_UNIT_TEST(eq_test 1 0x8000)
ADD_UNIT_TEST(include_test 1 0x8000)

ADD_UNIT_TEST(isa_v2_test 2 0x8000)

ADD_UNIT_TEST(cond_defs_1 1 0x8000)
ADD_UNIT_TEST(cond_defs_2 1 0x8000)
ADD_UNIT_TEST(cond_defs_3 2 0x8000)