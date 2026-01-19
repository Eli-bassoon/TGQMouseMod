.set noreorder
.global applyQueuedSpinAsm
.type applyQueuedSpinAsm, @function
applyQueuedSpinAsm:
    move $s3,$a0 /* Save a0 */

    jal testApplyQueuedSpin
    nop

    move $a0,$s3 /* Restore a0 */

    /* Original code */
    jal 0x00134f70 /* kcCGameSystem::GetInstance */
    move $s3,$a0

    /* Jump back */
    j 0x00161080
    nop
