

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
    InstructionDefs.cpp

    CpuModel.cpp
    CpuProgram.cpp

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


file(STRINGS InstructionDefs.txt IDEF_LIST)

set(DEF_STR "")
set(REG_STR "")
set(SUM_STR "")

string(ASCII 27 Esc)
set(InstructionColor    "${Esc}[1;36m")

message("${InstructionColor} Loading available SPECT instructions:${ColourReset}")
foreach(LINE ${IDEF_LIST})
    if (${LINE} MATCHES "^[RIJM]")
        separate_arguments(LINE_LIST UNIX_COMMAND ${LINE})

        list(GET LINE_LIST 0 ITYPE)
        list(GET LINE_LIST 1 MNEMONIC)
        list(GET LINE_LIST 2 OPCODE)
        list(GET LINE_LIST 3 FUNC)

        message("   ${InstructionColor}${MNEMONIC}${ColourReset}")

        # Instruction definitions (header + implicit constructor)
        set(DEF_STR "${DEF_STR} DEFINE_${ITYPE}_INSTRUCTION(Instruction${MNEMONIC},\"${MNEMONIC}\",${OPCODE},${FUNC}) ")

        # Instruction registration within instruction factory
        set(REG_STR "${REG_STR} REGISTER_${ITYPE}_INSTRUCTION(spect::Instruction${MNEMONIC}) ")

        # Instruction class forward definition
        set(SUM_STR "${SUM_STR} class Instruction${MNEMONIC}\\\; ")

    endif()
endforeach()

#message(${SUM_STR})
#message(${REG_STR})

target_compile_definitions(SPECT PUBLIC SPECT_DEFINE_INSTRUCTIONS=${DEF_STR} SPECT_REGISTER_INSTRUCTIONS=${REG_STR} SPECT_SUM_INSTRUCTIONS=${SUM_STR})
