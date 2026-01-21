.set noreorder
.global orbitCameraAsm
.type orbitCameraAsm, @function
orbitCameraAsm:
    jal orbitCamera
    move $a0, $s1
    j 0x001764b0
    nop
