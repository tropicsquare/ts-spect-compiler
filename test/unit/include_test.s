
_start:
    NOP

.include dummy_piece.s
    ADDI r0,r1, const_in_included_file
    END
