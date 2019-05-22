//###########################################################################
// $TI Release: DSP281x C/C++ Header Files V1.20 $
// $Release Date: July 27, 2009 $
//###########################################################################

#include "global.h"
#include "main.h"

//#define             FLASH
// Functions that will be run from RAM need to be assigned to
// a different section.  This section will then be mapped using
// the linker cmd file.
// #pragma CODE_SECTION(eva_timer1_isr, "ramfuncs");
// These are defined by the linker (see F2812.cmd)
#ifdef  FLASH

extern Uint16 RamfuncsLoadStart;
extern Uint16 RamfuncsLoadEnd;
extern Uint16 RamfuncsRunStart;

#endif


// Global variables
Uint16 sdataA[8];    // Send data for SCI-A
Uint16 sdataB[8];    // Send data for SCI-B
Uint16 rdataA[8];    // Received data for SCI-A
Uint16 rdataB[8];    // Received data for SCI-A
Uint16 rdata_pointA; // Used for checking the received data
Uint16 rdata_pointB;


void main(void)
{
    Uint16 i;
    float acc[4];
    memset(acc, 0.0f, 4);
    // Step 1. Initialize System Control:
    // PLL, WatchDog, enable Peripheral Clocks
    // This example function is found in the DSP281x_SysCtrl.c file.
    InitSysCtrl();
    EALLOW;
    SysCtrlRegs.HISPCP.all = 0x03;
    EDIS;
    // Step 2. Initalize GPIO:
    // This example function is found in the DSP281x_Gpio.c file and
    // illustrates how to set the GPIO to it's default state.
    InitGpio();  // Skipped for this example

    // Step 3. Clear all interrupts and initialize PIE vector table:
    // Disable CPU interrupts
    DINT;

    // Initialize PIE control registers to their default state.
    // The default state is all PIE interrupts disabled and flags
    // are cleared.
    // This function is found in the DSP281x_PieCtrl.c file.
    InitPieCtrl();

    // Disable CPU interrupts and clear all CPU interrupt flags:
    IER = 0x0000;
    IFR = 0x0000;

    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    // This will populate the entire table, even if the interrupt
    // is not used in this example.  This is useful for debug purposes.
    // The shell ISR routines are found in DSP281x_DefaultIsr.c.
    // This function is found in DSP281x_PieVect.c.
    InitPieVectTable();

    // Interrupts that are used in this example are re-mapped to
    // ISR functions found within this file.
    EALLOW;  // This is needed to write to EALLOW protected registers
    PieVectTable.RXAINT = &sciaRxFifoIsr;
    PieVectTable.RXBINT = &scibRxFifoIsr;
    EDIS;   // This is needed to disable write to EALLOW protected registers


    // Step 4. Initialize all the Device Peripherals:
    // This function is found in DSP281x_InitPeripherals.c
    // InitPeripherals(); // Not required for this example
    SCI_INIT();
    InitAdc();
    // Step 5. User specific code, enable interrupts:

    // Init send data.  After each transmission this data
    // will be updated for the next transmission
    for(i = 0; i<8; i++) {
        sdataA[i] = i;
    }

    for(i = 0; i<8; i++) {
        sdataB[i] = 0xFF - i;
    }
    rdata_pointA = sdataA[0];
    rdata_pointB = sdataB[0];
    // Copy time critical code and Flash setup code to RAM
    // This includes the following ISR functions: EvaTimer1(), EvaTimer2()
    // EvbTimer3 and and InitFlash();
    // The  RamfuncsLoadStart, RamfuncsLoadEnd, and RamfuncsRunStart
    // symbols are created by the linker. Refer to the F2812.cmd file.
#ifdef  FLASH
    MemCopy(&RamfuncsLoadStart, &RamfuncsLoadEnd, &RamfuncsRunStart);

    // Call Flash Initialization to setup flash waitstates
    // This function must reside in RAM
    InitFlash();
#endif
    // Enable interrupts required for this example
    PieCtrlRegs.PIECRTL.bit.ENPIE   = 1;   // Enable the PIE block
    PieCtrlRegs.PIEIER9.bit.INTx1   = 1;     // PIE Group 9, INT1
    PieCtrlRegs.PIEIER9.bit.INTx2   = 1;     // PIE Group 9, INT1
    PieCtrlRegs.PIEIER9.bit.INTx3   = 1;     // PIE Group 9, INT3
    PieCtrlRegs.PIEIER9.bit.INTx4   = 1;     // PIE Group 9, INT1
    PieCtrlRegs.PIEIER1.bit.INTx6   = 0;
    IER |= M_INT9; // Enable CPU INT

    ERTM;   // Enable Global realtime interrupt DBGM
    // Enable global Interrupts and higher priority real-time debug events:
    EINT;   // Enable Global interrupt INTM
    esp8266_init();
    DELAY_MS(9000);
    AdcRegs.ADCTRL1.bit.ACQ_PS = 0x01;
    AdcRegs.ADCTRL1.bit.SEQ_CASC = 0;
    AdcRegs.ADCTRL3.bit.SMODE_SEL = 0;
    AdcRegs.ADCMAXCONV.all = 0x0032;
    AdcRegs.ADCCHSELSEQ1.bit.CONV00 = 0x0;
    AdcRegs.ADCCHSELSEQ1.bit.CONV01 = 0x1;
    AdcRegs.ADCCHSELSEQ1.bit.CONV02 = 0x2;
    AdcRegs.ADCCHSELSEQ1.bit.CONV03 = 0x3;

    AdcRegs.ADCTRL2.bit.EVA_SOC_SEQ1 = 0;
    AdcRegs.ADCTRL2.bit.SOC_SEQ1 = 1;

    AdcRegs.ADCTRL2.all = 0x2000;
    // Step 6. IDLE loop. Just sit and loop forever:
    while(1) {
        if (is_enable_send == true) {
            esp8266_write_to_host(acc);
            DELAY_MS(500);
        }
        while(AdcRegs.ADCST.bit.INT_SEQ1 == 0);
        AdcRegs.ADCST.bit.INT_SEQ1_CLR = 1;
        acc[0] = (AdcRegs.ADCRESULT0 >> 4) * 3.3f ;
        acc[1] = (AdcRegs.ADCRESULT1 >> 4) * 3.3f ;
        acc[2] = (AdcRegs.ADCRESULT2 >> 4) * 3.3f ;
        acc[3] = (AdcRegs.ADCRESULT3 >> 4) * 3.3f ;
        AdcRegs.ADCTRL2.all = 0x2000;
        // sight in console.
#ifndef FLASH
        //printf("debug: Hello World!!!\n");
#endif
        //scia_msg("scia: Hello World!!! \n\r");
        //scib_msg("scib: Hello World!!! \n\r");
//        DELAY_MS( 1000 );

    }

}

//===========================================================================
// No more.
//===========================================================================
