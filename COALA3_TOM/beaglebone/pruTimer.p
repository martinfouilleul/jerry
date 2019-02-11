//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      @ Ircam 2014
//      pruTimer.p
//      ASM PRU code
//      programmed by Robert Piechaud
//

.origin 0
.entrypoint START

#include "PruEssentials.hp" 

START:
    // reset the time and clock value in register r0+r1 and shared pru ram:
    // BASE_ADDRESS_REG (def) stores the base add. for data exchange (typically: shared pru ram):
    MOV BASE_ADDRESS_REG, MEMORY_LOCAL_SHAREDRAM
    // r0+r1 = time (64 bits)
    // r3 = clock delay, r4 = delay counter, r5 = clock state:
    ZERO 0, 24	//clears r0 to r5
    SBBO r0, BASE_ADDRESS_REG, OFFSET_TIME64, 8

BIG_LOOP:
    ADD r0, r0, 1
    QBEQ INC_HIGH_BYTES, r0, 0
    JMP CONTINUE1
INC_HIGH_BYTES:
    ADD r1, r1, 1
CONTINUE1:
    NOP
    // update the time value in shared pru ram:
    SBBO r0, BASE_ADDRESS_REG, OFFSET_TIME64, 8
    //update the cyclical clock phase (1/0)
CHANGE_PHASE:
CONTINUE2:
    JMP BIG_LOOP    

EXIT:
    MOV R31.b0, PRU0_ARM_INTERRUPT+16
    HALT
