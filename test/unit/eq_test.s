
_start:
    NOP
    const_1 .eq 0x12
    NOP
    const_2 .eq 0xFFF
    NOP

dummy_label:
    ADD r0,r1,r2
    ADDI r0,r1, const_1
    JMP const_1
    LD r25, const_2
    NOP
    NOP
    SUBI r0,r1,const_defined_later
    NOP
    const_defined_later .eq 145
    NOP
    NOP
    JMP label_defined_later
    NOP
    NOP
label_defined_later:
    END
