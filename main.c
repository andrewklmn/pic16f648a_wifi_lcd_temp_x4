/*
 * File:   main.c
 * Author: user
 *
 * Created on 15 березня 2017, 22:00
 */

#define _XTAL_FREQ 4000000      // тактовая частота 

#define RS RB0                  // порты для подключения LCD
#define EN RB3
#define D4 RB4
#define D5 RB5
#define D6 RB6
#define D7 RB7

#define RESET_WIFI_STATE TRISA4 // ресет модуля вай-фай 
#define RESET_WIFI_PIN RA4 

// CONFIG
#pragma config FOSC = INTOSCIO  // Oscillator Selection bits (INTOSC oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RA5/MCLR/VPP Pin Function Select bit (RA5/MCLR/VPP pin function is digital input, MCLR internally tied to VDD)
#pragma config BOREN = OFF      // Brown-out Detect Enable bit (BOD disabled)
#pragma config LVP = OFF        // Low-Voltage Programming Enable bit (RB4/PGM pin has digital I/O function, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EE Memory Code Protection bit (Data memory code protection off)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <pic16f648a.h>
#include <stdio.h>
#include <stdlib.h>
#include <pic16f628a.h>
#include "lcd.h"
#include "usart.h"
#include "onewire.h"


char *aa[17];
char a[16] = "";

char c;             // это символ для считывания из UART
int i = 0;          // счётчик символа для комманд из UART
signed int t = 0;   // переменная для хранения температуры
char j = 0;         // счётчик для датчиков температуры

void main(void) {
    
    INTCON = 0b11000000;; //разрешить прерывания от периферии
    RCIE=1;    // разрешаем прерывание по приему байта UART
    CMCON = 0b111; //Disable PORTA Comparators
    
    
    TRISB = 0b00000010;
    RESET_WIFI_STATE = 0; // Переключаем на ВЫХОД ногу для ресета ВАЙ-ФАЙ
    RESET_WIFI_PIN = 0;   // начинаем ресет вайфая
    Lcd_Init();
    init_comms();   // старт UART
    __delay_ms(500);
    
    //print_to_uart("System start");
    
    Lcd_Clear();
    Lcd_Set_Cursor(1,1);
    Lcd_Write_String("Temp: ");
    Lcd_Set_Cursor(2,1);
    Lcd_Write_String("min      max");  
    RESET_WIFI_PIN = 1; // заканчиваем ресет вай-фая
    
    TRISAbits.TRISA2 = 0; // переключаем RA2 на выход
    
    
    while(1){
        // считываем датчики в цикле
        for (j=0; j<4; j++) {
            t = get_temp(j);
            if (nosensor==1) {
                printf("temp%d = ---\r\n", j ); 
            } else {
                if ( t < 0 ) {
                    printf("temp%d = %d,%dC\r\n", j , t/10, -(t - t/10*10));
                } else {
                    printf("temp%d = +%d,%dC\r\n", j , t/10, t - t/10*10);
                };
            };
        };
        __delay_ms(300);
    };
    return;
}

void interrupt isr(void) {
    
    if ((RCIE)&&(RCIF)) { // если что-то пришло в приёмник UART и принимать можно
        if(!RX9D && !OERR && !FERR) {   // если нет ошибок
            c = RCREG;
            if ( c  == 0x0d || c  == 0x0a ) { //введен символ конца строки или возврат каретки
                a[i] = '\0';
                // Начинаем распознавать и выполнять комманды, которые пришли
                if (a[0]=='A' && a[1]=='T') { //если это комманда, то обрабатываем   
                    if (a[2]=='\0') { // тестовая комманда которая возвращает просто ОК
                        print_to_uart("OK");
                    };
                    if (a[2]==' ' && a[3]=='1' && a[4]=='\0') { //AT 1\0  - текущая температура
                        print_to_uart("1");
                    };
                }; 
                // если другой набор данных - игнорируем и не отвечаем
                a[0] = '\0'; //записываем пустую строку
                i = 0;       //сбрасываем указатель символа на 0
            } else {
                //Заполняем буффер следующим символом
                if (i==16){ // если переполнение то сбрасываем на первый символ
                    a[0] = c;
                    i = 0;
                } else { //увеличиваем команду на 1 единицу
                    a[i] = c;
                    i++;
                };
            };
            
        } else {
            //сброс ошибки приёмника
            c = RCREG;
            c = RCREG;
            CREN = 0;
            NOP();
            NOP();
            CREN = 1;
        };
    };
};