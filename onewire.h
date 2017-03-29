#ifndef _ONEWIRE_H
#define _ONEWIRE_H_

#define STATE TRISAbits.TRISA0   // порты датчиков температуры
#define PIN PORTAbits.RA0

unsigned char nosensor;                 // флаг отсутствия сенсора

void TX(unsigned char cmd, unsigned char ds);
unsigned char RX(unsigned char ds);
signed int get_temp(unsigned char ds);

#endif