.setcallreg r29.w0

.origin 0
.entrypoint GPIO_TEST 

#include "pru.hp"

GPIO_TEST:
    CLR DAC_DIN
    SET DAC_SCLK
    SET DAC_SYNC
    CLR ADC_SCLK
    CLR ADC_CONVST

    CALL DELAY

AGAIN:
    MOV r10, 65535 // loop r10 times
    MOV r20, 0
MAINLOOP:
    CALL ADCIN
    MOV r20.w0, r21.w0
    //MOV r20, r10
    CALL DACOUT
    SUB r10, r10, 1
    QBNE MAINLOOP, r10, 0
    QBA AGAIN

    // Send notification to Host for program completion
    MOV r31.b0, PRU1_ARM_INTERRUPT+16

    // Halt the processor
    HALT

// 1ms delay
DELAY:
    MOV r0, 100000
DELBCL:
    SUB r0, r0, 1
    QBNE DELBCL, r0, 0
    RET

// short delay
DELAY2:
    MOV r0, 2
DELBCL2:
    SUB r0, r0, 1
    QBNE DELBCL2, r0, 0
    RET

// get ADC value in r21 via SPI
ADCIN:
    MOV r2, 16
    MOV r21, 0
    SET ADC_CONVST
    NOP
    NOP
    NOP
    NOP
    CLR ADC_CONVST
POLLEOC:
    SET ADC_SCLK
    NOP
    NOP
    NOP
    NOP
    CLR ADC_SCLK
    NOP
    NOP
    NOP
    NOP
    QBBS POLLEOC, r31, 9
GETRES:
    SET ADC_SCLK
    NOP
    NOP
    NOP
    NOP
    CLR ADC_SCLK
    NOP
    NOP
    NOP
    SUB r1, r2, 1
    QBBC INBITCLR, r31, 9
    SET r21, r21, r1
INBITCLR:
    SUB r2, r2, 1
    QBNE GETRES, r2, 0
    RET

// write 24 bits of r20 on DAC via SPI
DACOUT:
    MOV r2, 24
    CLR DAC_SYNC
DACBCL:
    SUB r1, r2, 1
    QBBS BITSET, r20, r1
    CLR DAC_DIN
    QBA CLOCK  
BITSET:
    SET DAC_DIN
CLOCK:
    NOP
    CLR DAC_SCLK
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    SET DAC_SCLK
    SUB r2, r2, 1
    QBNE DACBCL, r2, 0
    SET DAC_SYNC
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    NOP
    RET
    


