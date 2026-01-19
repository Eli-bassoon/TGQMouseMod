.set noreorder
.global lookAtFroggerAsm
.type lookAtFroggerAsm, @function
lookAtFroggerAsm:
    lwc1 $f3,0x290($s0)
    swc1 $f3,0xb0($sp)
    /*lwc1 $f3,0x294($s0)*/
    lwc1 $f3,cameraFocusY
    swc1 $f3,0xb4($sp)
    lwc1 $f3,0x298($s0)
    j 0x001754b8
    swc1 $f2,0xc0($sp)
