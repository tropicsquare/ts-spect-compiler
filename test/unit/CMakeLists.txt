

add_test(NAME r_type_unit_test COMMAND spect_compiler --first-address=0x8000
                                                      --sim-hex=${CMAKE_CURRENT_SOURCE_DIR}/r_type_test.hex
                                                      ${CMAKE_CURRENT_SOURCE_DIR}/r_type_test.s)

add_test(NAME i_type_unit_test COMMAND spect_compiler --first-address=0x8000
                                                      --sim-hex=${CMAKE_CURRENT_SOURCE_DIR}/i_type_test.hex
                                                      ${CMAKE_CURRENT_SOURCE_DIR}/i_type_test.s)

add_test(NAME m_type_unit_test COMMAND spect_compiler --first-address=0x8000
                                                      --sim-hex=${CMAKE_CURRENT_SOURCE_DIR}/m_type_test.hex
                                                      ${CMAKE_CURRENT_SOURCE_DIR}/m_type_test.s)

add_test(NAME j_type_unit_test COMMAND spect_compiler --first-address=0x8000
                                                      --sim-hex=${CMAKE_CURRENT_SOURCE_DIR}/j_type_test.hex
                                                      ${CMAKE_CURRENT_SOURCE_DIR}/j_type_test.s)