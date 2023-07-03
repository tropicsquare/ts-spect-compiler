_start:

; Both macros definedd
.define MACRO_A1
.define MACRO_B1

.ifdef MACRO_A1
    .ifdef MACRO_B1
        NOP
        NOP
        NOP
        NOP
    .else
        NOP
        NOP
        NOP
    .endif
.else
    .ifdef MACRO_B1
        NOP
        NOP
    .else
        NOP
    .endif
.endif


; Only first macro defined
.define MACRO_A2
; .define MACRO_B2

.ifdef MACRO_A2
    .ifdef MACRO_B2
        ADD r1,r2,r3
        ADD r1,r2,r3
        ADD r1,r2,r3
        ADD r1,r2,r3
    .else
        ADD r1,r2,r3
        ADD r1,r2,r3
        ADD r1,r2,r3
    .endif
.else
    .ifdef MACRO_B2
        ADD r1,r2,r3
        ADD r1,r2,r3
    .else
        ADD r1,r2,r3
    .endif
.endif


; Only second macro defined
; .define MACRO_A3
.define MACRO_B3

.ifdef MACRO_A3
    .ifdef MACRO_B3
        SUB r1,r2,r3
        SUB r1,r2,r3
        SUB r1,r2,r3
        SUB r1,r2,r3
    .else
        SUB r1,r2,r3
        SUB r1,r2,r3
        SUB r1,r2,r3
    .endif
.else
    .ifdef MACRO_B3
        SUB r1,r2,r3
        SUB r1,r2,r3
    .else
        SUB r1,r2,r3
    .endif
.endif



; No macro defined
; .define MACRO_A4
; .define MACRO_B4

.ifdef MACRO_A4
    .ifdef MACRO_B4
        SUB r10,r11,r12
        SUB r10,r11,r12
        SUB r10,r11,r12
        SUB r10,r11,r12
    .else
        SUB r10,r11,r12
        SUB r10,r11,r12
        SUB r10,r11,r12
    .endif
.else
    .ifdef MACRO_B4
        SUB r10,r11,r12
        SUB r10,r11,r12
    .else
        SUB r10,r11,r12
    .endif
.endif