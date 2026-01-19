.set noreorder
.global orbitCameraAsm
.type orbitCameraAsm, @function
orbitCameraAsm:
    move $a0, $s1
    jal orbitCamera
    move $a1, $s0
    j 0x001764b0
    nop
