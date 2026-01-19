.set noreorder
.global freelookCameraAsm
.type freelookCameraAsm, @function
freelookCameraAsm:
    jal freelookCamera
    nop
    j 0x001758c8
    nop
