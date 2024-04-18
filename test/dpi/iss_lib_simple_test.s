; ==============================================================================
;	file: instr_mem_fw_test.s
;   author: Vit Masek
;
;   license: TODO
;
; ==============================================================================
;
;    Moves number 3070 to r0.
;    Substract 1 from r0 3069 times.
;    Stores r0 to 0x1000 (r0 should equal 1)
;    Ends
;
;==============================================================================

_start:
    MOVI r0, 3070
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    SUBI r0, r0, 1
    ST      r0, 0x1000
    END
