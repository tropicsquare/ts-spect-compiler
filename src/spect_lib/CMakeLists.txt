

add_library (SPECT STATIC
    Compiler.cpp
    Symbol.cpp
    SymbolTable.cpp
    SourceFile.cpp

    Instruction.cpp

    InstructionR.cpp
    InstructionI.cpp
    InstructionM.cpp
    InstructionJ.cpp

    InstructionFactory.cpp
    InstructionDefsV1.cpp
    InstructionDefsV2.cpp

    CpuModel.cpp
    CpuProgram.cpp
    CpuSimulator.cpp

    KeyMemory.cpp

    HexHandler.cpp

    ordt_pio_common.cpp
    ordt_pio.cpp

    spect.cpp
)

function(string_starts_with str search)
  string(FIND "${str}" "${search}" out)
  if("${out}" EQUAL 0)
    return(true)
  endif()
  return(false)
endfunction()

###################################################################################################
# Load all available ISA versions
###################################################################################################
LIST (APPEND ISA_LIST InstructionDefs_v1)
LIST (APPEND ISA_LIST InstructionDefs_v2)

foreach (ISA_TEMPLATE ${ISA_LIST})

  file(STRINGS ${TS_SPECT_COMPILER_ROOT}/templates/${ISA_TEMPLATE}.txt IDEF_LIST)

  set(DEF_STR "")
  set(REG_STR "")
  set(SUM_STR "")

  list(GET IDEF_LIST 0 FIRST_LINE)
  separate_arguments(FIRST_LINE_LIST UNIX_COMMAND ${FIRST_LINE})
  list(GET FIRST_LINE_LIST 1 ISA_VERSION)

  string(ASCII 27 Esc)
  set(ColourReset         "${Esc}[m")
  set(InstructionColor    "${Esc}[1;36m")

  message("${InstructionColor} Loading SPECT instructions from ISA version ${ISA_VERSION}:${ColourReset}")
  foreach(LINE ${IDEF_LIST})
      if (${LINE} MATCHES "^[RIJM]")
          separate_arguments(LINE_LIST UNIX_COMMAND ${LINE})

          list(GET LINE_LIST 0 ITYPE)
          list(GET LINE_LIST 1 MNEMONIC)
          list(GET LINE_LIST 2 OPCODE)
          list(GET LINE_LIST 3 FUNC)
          list(GET LINE_LIST 4 OPMASK)
          list(GET LINE_LIST 5 R31_DEP)
          list(GET LINE_LIST 6 C_TIME)
          list(GET LINE_LIST 7 CROSS_COVER)
          list(GET LINE_LIST 8 CYCLES)

          message("   ${InstructionColor}${MNEMONIC}${ColourReset}")

          # Instruction definitions (header + implicit constructor)
          set(DEF_STR "${DEF_STR} DEFINE_${ITYPE}_INSTRUCTION(V${ISA_VERSION}Instruction${MNEMONIC},\"${MNEMONIC}\",${OPCODE},${FUNC},${OPMASK},${R31_DEP},${C_TIME},${CYCLES}) ")

          # Instruction registration within instruction factory
          set(REG_STR "${REG_STR} REGISTER_${ITYPE}_INSTRUCTION(${ISA_VERSION},spect::V${ISA_VERSION}Instruction${MNEMONIC}) ")

          # Instruction class forward definition
          set(SUM_STR "${SUM_STR} class V${ISA_VERSION}Instruction${MNEMONIC}\\\; ")

      endif()
  endforeach()

  #message(${SUM_STR})
  #message(${REG_STR})

  target_compile_definitions(SPECT PUBLIC
                             SPECT_DEFINE_INSTRUCTIONS_V${ISA_VERSION}=${DEF_STR}
                             SPECT_REGISTER_INSTRUCTIONS_V${ISA_VERSION}=${REG_STR}
                             SPECT_SUM_INSTRUCTIONS_V${ISA_VERSION}=${SUM_STR})

endforeach()


# Waivers
set_source_files_properties(InstructionDefs.cpp PROPERTIES COMPILE_FLAGS -Wno-unused-parameter)
set_source_files_properties(ordt_pio_common.cpp PROPERTIES COMPILE_FLAGS -Wno-type-limits)
set_source_files_properties(ordt_pio.cpp PROPERTIES COMPILE_FLAGS -Wno-unused-parameter)
set_source_files_properties(CpuModel.cpp PROPERTIES COMPILE_FLAGS -Wno-delete-non-virtual-dtor)
