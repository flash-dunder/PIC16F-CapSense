#include "serial.h"


void UART_Init(const long int baudrate) {
  TRISC6 = 0;   // Output Tx PIN
  TRISC7 = 1;   // Input Rx PIN

  BAUDCON = 0b00001000;
  TXSTA = 0b00100100;
  RCSTA = 0b10010000;

  unsigned int baud_select = ((( _XTAL_FREQ / baudrate) / 4) - 1);
  SPBRGH = (baud_select >> 8);
  SPBRGL = (baud_select);
}

void UART_Write(char data) {
  while(!TRMT);
  TXREG = data;
}

void UART_Write_Text(char *myString) {
  for(int i=0; myString[i]!='\0'; i++)
    UART_Write(myString[i]);
}

char UART_Read(void) {
  if(OERR) {
      CREN = 0; //If error -> Reset 
      CREN = 1; //If error -> Reset 
  }
  while(!RCIF);
  return RCREG;
}

void UART_Read_Text(char* Output, unsigned int length) {
  for(int i=0; i<length; i++)
  Output[i] = UART_Read();
}
