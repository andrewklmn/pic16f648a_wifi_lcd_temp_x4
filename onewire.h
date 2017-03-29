#ifndef _ONEWIRE_H
#define _ONEWIRE_H_

// Предполагается что датчики подключены на RA0-RA3


unsigned char nosensor;                 // флаг отсутствия сенсора

void TX(unsigned char cmd, unsigned char ds);
unsigned char RX(unsigned char ds);
signed int get_temp(unsigned char ds); //возвращает температуру на датчике номер ds 0-3

#endif