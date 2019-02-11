//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      @ Ircam 2014
//      pruConverter.p
//      ASM PRU code
//
//      This file implements the communication with TI's DAC DAC8551 & ADC ADS8860
//		(16bit converters) in PRU1.
//

#include "PruEssentials.hp"

// set the call return addr register
.setcallreg r29.w0

.origin 0
.entrypoint INIT 

INIT:
    CLR DAC_DIN
    SET DAC_SCLK
    SET DAC_SYNC
    CLR ADC_SCLK
    CLR ADC_CONVST

    MOV R_OUTPUTVALUE, 0
    
	MOV r0,0
	SBCO r0, CONST_PRUDRAM, OFFSET_DATAIN_REQUEST, 4
	SBCO r0, CONST_PRUDRAM, OFFSET_DATAOUT_READY, 4
	SBCO r0, CONST_PRUDRAM, OFFSET_DATAOUT_DONE, 4
	SBCO r0, CONST_PRUDRAM, OFFSET_DATAIN_READY, 4

MAINLOOP:
    LBCO R_DATAIN_REQUEST, CONST_PRUDRAM, OFFSET_DATAIN_REQUEST, 4// Load 1 byte from mem loc (PRU0/1 Local Data)+ ADDRESS OFFSET DATAIN_REQUEST into DATAIN_REQUEST REGISTER
    QBBC STEP2, R_DATAIN_REQUEST.t0
    CALL ADCIN
    // apres le call precedent, R_INPUTVALUE contient la valeur convertie:
	  SBCO R_INPUTVALUE, CONST_PRUDRAM, OFFSET_DATAIN, 2
	  MOV R_DATAIN_READY, 1 // data is ready
	  SBCO R_DATAIN_READY, CONST_PRUDRAM, OFFSET_DATAIN_READY, 4 // store value of DATAIN_READY to the memory location OFFSET_DATAIN_READY ====> data not ready for the model

WAIT_FOR_REQUEST_DOWN:
    LBCO R_DATAIN_REQUEST, CONST_PRUDRAM, OFFSET_DATAIN_REQUEST, 4// Load 1 byte from mem loc (PRU0/1 Local Data)+ ADDRESS OFFSET DATAIN_REQUEST into DATAIN_REQUEST REGISTER
    QBBS WAIT_FOR_REQUEST_DOWN, R_DATAIN_REQUEST.t0
    MOV R_DATAIN_READY, 0
	  SBCO R_DATAIN_READY, CONST_PRUDRAM, OFFSET_DATAIN_READY, 4

STEP2:
    LBCO R_DATAOUT_READY, CONST_PRUDRAM, OFFSET_DATAOUT_READY, 4// Load 1 byte from mem loc (PRU0/1 Local Data)+ ADDRESS OFFSET DATAIN_REQUEST into DATAIN_REQUEST REGISTER
    QBBC STEP3, R_DATAOUT_READY.t0
    LBCO R_OUTPUTVALUE, CONST_PRUDRAM, OFFSET_DATAOUT, 2 // set to register the output from memory
    MOV R_DATAOUT_DONE, 1
    SBCO R_DATAOUT_DONE, CONST_PRUDRAM, OFFSET_DATAOUT_DONE, 4 // set the memory from 'done' register 
    CALL DACOUT

WAIT_FOR_READY_DOWN:
    LBCO R_DATAOUT_READY, CONST_PRUDRAM, OFFSET_DATAOUT_READY, 4
    QBBS WAIT_FOR_READY_DOWN, R_DATAOUT_READY.t0
    MOV R_DATAOUT_DONE, 0
    SBCO R_DATAOUT_DONE, CONST_PRUDRAM, OFFSET_DATAOUT_DONE, 4 // set the memory from 'done' register 

STEP3:
    QBA MAINLOOP


// 1ms delay
DELAY:
    MOV r0, 100000
DELBCL:
    SUB r0, r0, 1
    QBNE DELBCL, r0, 0
    RET


// get ADC value in R_INPUTVALUE via SPI
ADCIN:
    MOV r2, 16
    //R_INPUTVALUE = resultat
    MOV R_INPUTVALUE, 0
    SET ADC_CONVST
    NOP
    NOP
    NOP
    NOP
    CLR ADC_CONVST
POLL_EOC:
    // r31.9 = data in / pru = busy dans cette phase, on attend que ce soit a zero 
    QBBS POLL_EOC, r31, 9
GETRES:
    CLR ADC_SCLK
    NOP
    NOP
    NOP
    NOP
    SET ADC_SCLK
    NOP
    NOP
    NOP
    // r1 = r2 -1 == num. du bit (0...15)
    SUB r1, r2, 1
    QBBC INBITCLR, r31, 9
    SET R_INPUTVALUE, R_INPUTVALUE, r1
INBITCLR:
    SUB r2, r2, 1
    QBNE GETRES, r2, 0
    CLR ADC_SCLK
    NOP
    NOP
    NOP
    NOP
    SET ADC_SCLK
    RET

// write 24 bits of R_OUTPUTVALUE on DAC via SPI
DACOUT:
    MOV r2, 24
    CLR DAC_SYNC
DACBCL:
    SUB r1, r2, 1
    // R_OUTPUTVALUE = valeur a envoyer
    QBBS BITSET, R_OUTPUTVALUE, r1
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
    RET
