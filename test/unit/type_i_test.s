
_start:
    ; 32 bit arithmetic
    ADDI r1,r2,0x12
    SUBI r4,r5, 456       ; Example comment
    CMPI r7, 0xFFF

    ; 32 bit logic
    ANDI r9, r10,0x22
    ORI r12,r13,0x79
    XORI r15, r16,0x47
    CMPA r18, 0xAAA

    ; OTHER
    MOVI r19, 0xFFF
    HASH_IT
    GPK r31, 0x123