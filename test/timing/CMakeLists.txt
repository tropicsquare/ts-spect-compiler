
macro(ADD_TIMING_TEST TEST_NAME MIN_EXP_TIME MAX_EXP_TIME)
    add_test(NAME ${TEST_NAME} COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/check_duration.sh ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}.s ${MIN_EXP_TIME} ${MAX_EXP_TIME})
endmacro()

# Expected time
# loop_count * (MULP+SUBI+BRNZ) * time_step
# 100        * (597 + 11 +  5 ) * 200ns = 12260000ns = 12.26s
ADD_TIMING_TEST(mulp_loop_test 12.0 13.0)
