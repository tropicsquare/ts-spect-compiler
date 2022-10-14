

_start:
    NOP
    NOP

go_back:
    NOP
    CALL go_back
    BRZ go_back
    BRNZ go_back
    BRC go_back
    BRNC _start