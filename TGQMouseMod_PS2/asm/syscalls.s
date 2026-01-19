.set noreorder
.global _print
.type _print, @function
_print:
    li $v1, 0x75
    syscall
    jr $ra
    nop
