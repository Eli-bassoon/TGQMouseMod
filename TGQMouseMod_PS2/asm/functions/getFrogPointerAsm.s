.set noreorder
.global getFrogPointerAsm
.type getFrogPointerAsm, @function
getFrogPointerAsm:
    sw $a0,frogPtr
    nop
    j 0x001625c8
    /* Original code */
    move $s2,$a0
