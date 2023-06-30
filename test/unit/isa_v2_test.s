
_start:
    MOVI r0,  0x000
    MOVI r1,  0x001
    MOVI r2,  0x002
    MOVI r3,  0x003
    MOVI r4,  0x004
    MOVI r5,  0x005
    MOVI r6,  0x006
    MOVI r7,  0x007
    MOVI r8,  0x008
    MOVI r9,  0x009
    MOVI r10, 0x00A
    MOVI r11, 0x00B
    MOVI r12, 0x00C
    MOVI r13, 0x00D
    MOVI r14, 0x00E
    MOVI r15, 0x00F
    MOVI r16, 0xFF0
    MOVI r17, 0xFF1
    MOVI r18, 0xFF2
    MOVI r19, 0xFF3
    MOVI r20, 0xFF4
    MOVI r21, 0xFF5
    MOVI r22, 0xFF6
    MOVI r23, 0xFF7
    MOVI r24, 0xFF8
    MOVI r25, 0xFF9
    MOVI r26, 0xFFA
    MOVI r27, 0xFFB
    MOVI r28, 0xFFC
    MOVI r29, 0xFFD
    MOVI r30, 0xFFE
    MOVI r31, 0xFFF

    ; Bit manipulation instructions
    SBIT r1, r2, r3
    CBIT r4, r5, r6

    ; Single bit shift
    ROLIN r7, r8, r9
    RORIN r10, r11, r12

    ; Load / Store -> Addressed from register
    LDR r1, r2
    STR r3, r4

    ; Additional swap operators
    ZSWAP r5, r6

    ; Branching absed on Error flag
    BRE _start
    BRNE _start

    ; TMAC
    TMAC_IT r10
    TMAC_IS r13, 0x34
    TMAC_UP r15
    TMAC_RD r16

    ; Additional Key instructions
    LDK r20, r21, 0x12
    STK r22, r23, 0x156
    KBO r24, 0x52
