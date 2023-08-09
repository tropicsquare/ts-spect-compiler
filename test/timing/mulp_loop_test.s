
_start:
    ; Execute MULP 100 times
    MOVI r1, 100
_loop:
    MULP r14, r15, r16
    SUBI r1, r1, 1
    BRNZ _loop
    END
