#include "main.h"



unsigned int CapSense(void){
    unsigned int tempADC = 0;
    /*----- iterations should be <64 to avoid stack overflow for tempADC -----*/
    int iterations = 16;
    for (int i=0; i<iterations; i++) {
        /*----- enable GPIO output low to discharge VChold -----*/
        TRISB5  = 0;
        ANSELB  = 0;
        RB5     = 0;
        /*----- enable ADC input to charge up VChold -----*/
        TRISB5  = 1;             // RB5 to input for ADC
        ANSELB  = 0b00100000;    // select AN13
        ADCON0  = 0b00110101;    // enable AN13 and ADC
        ADCON1  = 0b01010000;    // Fosc÷16, Vref- to Vss, Vref+ to AVdd
        /*----- start ADC conversion and sum results -----*/
        ADGO    = 1;
        while(ADGO);
        tempADC += (ADRESH << 8) + ADRESL;
    }
    return tempADC;
}


void main() {
    OSCCON = 0x78;

    UART_Init(115200);

    TRISD0 = 0;

    while(1) {
        unsigned int threshADC = CapSense();
        /*----- debug: send 2 byte ADC sum data thru UART -----*/
        UART_Write(threshADC>>8);
        UART_Write(threshADC);
        UART_Write(0xFF);   // pad serial data with 0xFFFF (2 bytes) delimiter
        UART_Write(0xFF);   // since ADC sum will never != 2^16 for <64 iterations
        /*----- enable RD0 LED -----*/
        if (threshADC>1024) {
            RD0 = 1;
            __delay_ms(100);
        } else {
            RD0 = 0;
        }
    }
}
