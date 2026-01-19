.set noreorder
.global transitionFreelookAsm
.type transitionFreelookAsm, @function
transitionFreelookAsm:
    lw $v0,0x1a4($s0)
    sw $v0,0x1a0($s0)
    lw $v0,0x1c4($s0)
    sw $v0,0x1c0($s0)
    lw $v0,0x1e4($s0)
    sw $v0,0x1e0($s0)

    /* Original code */
    lwc1 $f0,0x324($s0)
    j 0x001756f8
    lui $v0,0x4000
