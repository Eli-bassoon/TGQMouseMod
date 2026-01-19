.set noreorder
.global aimGoobersOrbitAsm
.type aimGoobersOrbitAsm, @function
aimGoobersOrbitAsm:
    /* Queue Frogger to move away from the camera next frame */
    li $v0,1
    sw $v0,queuedSpin

    /* Load the yaw into fp12 for the rotation matrix */
    lwc1 $f12,frogNewYaw

    /* Original code */
    jal 0x001ce880 /* Vu0RotMatrixY */
    move $a1,$a0

    /* Jump back */
    j 0x00166ad0
    nop
