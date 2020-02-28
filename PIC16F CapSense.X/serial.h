#ifndef SERIAL_H
#define SERIAL_H

#include "main.h"

void UART_Init(const long int baudrate);

void UART_Write(char data);

void UART_Write_Text(char *text);

char UART_Read(void);

void UART_Read_Text(char *Output, unsigned int length);

#endif