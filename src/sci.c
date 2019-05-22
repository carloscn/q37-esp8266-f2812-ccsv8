
#include "global.h"

bool scia_data_flag = false;
bool scib_data_flag = false;

unsigned char scia_buffer[50];
unsigned char scib_buffer[50];

uint16_t scia_rx_count = 0;
uint16_t scib_rx_count = 0;

void (*scia_char_signal)(char e);

void    SCI_INIT( void )
{
    EALLOW;
    GpioMuxRegs.GPFMUX.bit.SCITXDA_GPIOF4 = 1;
    GpioMuxRegs.GPFMUX.bit.SCIRXDA_GPIOF5 = 1;
    GpioMuxRegs.GPGMUX.bit.SCITXDB_GPIOG4 = 1;
    GpioMuxRegs.GPGMUX.bit.SCIRXDB_GPIOG5 = 1;
    EDIS;
    scia_fifo_init();
    scib_fifo_init();

}


void scia_xmit(int a)
{
    while( SciaRegs.SCICTL2.bit.TXRDY != 1 );
    SciaRegs.SCITXBUF=a & 0x00FF;

}

void scia_msg(char * msg)
{
    int i;
    i = 0;
    while(msg[i] != '\0')
    {
        scia_xmit(msg[i]);
        i++;

    }
}

void scib_xmit(int a)
{

    while( ScibRegs.SCICTL2.bit.TXRDY != 1 );
    ScibRegs.SCITXBUF=a & 0x00FF;

}
void scib_msg(char * msg)
{
    int i;
    i = 0;
    while(msg[i] != '\0')
    {
        scib_xmit(msg[i]);
        i++;
    }
}

void error(void)
{
    asm("     ESTOP0"); // Test failed!! Stop!
    for (;;);
}

uint16_t scia_read_all(unsigned char *buf)
{
    uint16_t len = scia_rx_count;

    memcpy(buf, scia_buffer, len);
    memset(scia_buffer, 0, len);
    scia_rx_count = 0;
    return len;

}

uint16_t scib_read_all(unsigned char *buf)
{

}

bool scia_is_have_data(void)
{
    return scia_is_have_data;
}

bool scib_is_have_data(void)
{

}
unsigned char sci_buffer[256];
long sci_count = 0;
extern void esp8266_read_signal(char e);
interrupt void sciaRxFifoIsr(void)
{
    Uint16 i;
    //scia_is_have_data = true;
    scia_char_signal(SciaRegs.SCIRXBUF.all&0xFF);
    //sci_buffer[sci_count++] = SciaRegs.SCIRXBUF.all& 0xFF;
    EINT;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
}

interrupt void scibRxFifoIsr(void)
{
    Uint16 i;
    for(i=0;i<8;i++)
    {
        //rdataB[i]=ScibRegs.SCIRXBUF.all;  // Read data
    }
    EINT;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
}


void scia_fifo_init()
{
    SciaRegs.SCICCR.all =0x0007;   // 1 stop bit,  No loopback
    // No parity,8 char bits,
    // async mode, idle-line protocol
    SciaRegs.SCICTL1.all =0x0003;  // enable TX, RX, internal SCICLK,
    // Disable RX ERR, SLEEP, TXWAKE
    SciaRegs.SCICTL2.bit.TXINTENA =1;
    SciaRegs.SCICTL2.bit.RXBKINTENA =1;

    // value = 37500000 / baud / 8 - 1

    SciaRegs.SCIHBAUD    =0x0000;  // 115200 baud @LSPCLK = 37.5MHz.
    SciaRegs.SCILBAUD    = 39;
    SciaRegs.SCICCR.bit.LOOPBKENA = 0; // Enable loop back
    SciaRegs.SCICTL2.bit.RXBKINTENA =   1;
    SciaRegs.SCICTL2.bit.TXINTENA   =   0;

    SciaRegs.SCICTL1.all =0x0023;     // Relinquish SCI from Reset
    scia_char_signal = &esp8266_read_signal;

}
void scib_fifo_init()
{
    ScibRegs.SCICCR.all =0x0007;   // 1 stop bit,  No loopback
    // No parity,8 char bits,
    // async mode, idle-line protocol
    ScibRegs.SCICTL1.all =0x0003;  // enable TX, RX, internal SCICLK,
    // Disable RX ERR, SLEEP, TXWAKE
    ScibRegs.SCICTL2.bit.TXINTENA =1;
    ScibRegs.SCICTL2.bit.RXBKINTENA =1;

    // value = 37500000 / baud / 8 - 1

    ScibRegs.SCIHBAUD    =0x0000;  // 115200 baud @LSPCLK = 37.5MHz.
    ScibRegs.SCILBAUD    = 39;
    ScibRegs.SCICCR.bit.LOOPBKENA = 0; // Enable loop back
    ScibRegs.SCICTL2.bit.RXBKINTENA =   1;
    ScibRegs.SCICTL2.bit.TXINTENA   =   0;

    ScibRegs.SCICTL1.all =0x0023;     // Relinquish SCI from Reset
}



