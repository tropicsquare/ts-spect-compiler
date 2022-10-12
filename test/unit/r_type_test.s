
_start:
    ; 32 bit arithmetic
    ADD r1,r2,r3
    SUB r4,r5, r6       ; Example comment
    CMP r7,r8

    ; 32 bit logic
    AND r9, r10,r11
    OR r12,r13,r14
    XOR r15, r16,r17
    NOT r18, r19

    ; Shift
    LSL r20, r21
    LSR r22, r23
    ROL r24, r25
    ROR r26, r27
    ROL8 r28, r29
    ROR8 r30, r31
    SWE r0, r1

    ; Modular arithmetic
    MUL25519 r2,r3,r4
    MUL256 r5,r6,r7
    ADDP r8,r9,r10
    SUBP r11,r12,r13
    MULP r14, r15, r16
    REDP r17, r18, r19

    ; Other
    MOV r20, r21
    CSWAP r22,r23
    HASH r24,r25
    GRV r26
    SCB r27,r28,r29