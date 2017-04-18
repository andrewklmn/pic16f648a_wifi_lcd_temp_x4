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
#pragma config MCLRE = ON      // RA5/MCLR/VPP Pin Function Select bit (RA5/MCLR/VPP pin function is digital input, MCLR internally tied to VDD)
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
#include <string.h>
#include "lcd.h"
#include "usart.h"
#include "onewire.h"

// переменные для хранения текущих данных по температуре
signed int  temp[4] = {0,0,0,0};               // текущие показания датчика
signed int tempmin[4] = {999,999,999,999};      // минимальные показания
signed int tempmax[4] = {-999,-999,-999,-999};  // максимальные показания
unsigned char active[4] = { 0,0,0,0 };          // массив активных датчиков 
unsigned char active_old[4] = { 1,1,1,1 };      // массив активных датчиков перед измерением


char a[16] = "";    // буффер для приема комманд по USART
char *aa[17];       // буффер для вывода цифр температуры

char c;             // это символ для считывания из UART
int i = 0;          // счётчик символа для комманд из UART
signed int t = 0;   // переменная для хранения температуры
char j = 0;         // счётчик для датчиков температуры


void draw_temp( int temp ){
        int zel = temp/10;
        int drob = temp - temp/10*10;
        if (temp < 0) {
            if (zel > 9) {
                sprintf( aa, "-%d.%d%c", ~(zel)+1, -(drob), 0xdf );
            } else {
                sprintf( aa, "-%d.%d%c ", ~(zel)+1, -(drob), 0xdf );
            }; 
        } else {
            if (temp >999)  {
                sprintf( aa,"+%d%c ", zel, 0xdf);
            } else {
                if (zel > 9) {
                    sprintf( aa, "+%d.%d%c", zel, drob, 0xdf );
                } else {
                    sprintf( aa, "+%d.%d%c ", zel, drob, 0xdf );
                }; 
            };
        };
        Lcd_Write_String(aa);    
};

void draw_screen(){
    
    if ( active[0]!=active_old[0] 
            || active[1]!=active_old[1] 
            || active[2]!=active_old[2] 
            || active[3]!=active_old[3] ) { // кол-во датчиков поменялось
        
        Lcd_Clear();
        active_old[0]=active[0];
        active_old[1]=active[1];
        active_old[2]=active[2];
        active_old[3]=active[3];
        
        if (active[2]==0 && active[3]==0) { // форматируем экран под режим двух датчиков 
            if (active[1]==1) {
                Lcd_Set_Cursor(1,1);
                Lcd_Write_String("Indoor:");
                Lcd_Set_Cursor(2,1);
                Lcd_Write_String("Outdoor:");
            } else {
                if ( active[0]==0 ) {
                    Lcd_Set_Cursor(1,1);
                    Lcd_Write_String("   No sensors!");
                } else {
                    Lcd_Set_Cursor(1,1);
                    Lcd_Write_String("Temp:");
                    Lcd_Set_Cursor(2,1);
                    Lcd_Write_String(">        <");
                };
            };
        } else {
                Lcd_Set_Cursor(1,1);
                Lcd_Write_String("1:      2:");            
                Lcd_Set_Cursor(2,1);
                Lcd_Write_String("3:      4:");            
        };
    };
    
    if ( active[2]==0 && active[3]==0 ) {   // только два датчика
        char dat = 1;
        
        if(active[0]==0 && active[1]==0) { // ввобще никаких датчиков
            __delay_ms(500);
        } else {
            if(active[1]==1) {
                dat = 2;
            } else {
                Lcd_Set_Cursor(2,2);
                draw_temp(tempmin[0]);
                Lcd_Set_Cursor(2,11);
                draw_temp(tempmax[0]);
                
            };
            for (j=0;j<dat;j++) {
                if (dat==1) {
                    Lcd_Set_Cursor(1,14);
                    if ( temp[0]>199 ) {
                        Lcd_Write_String(":-)");
                    } else {
                        Lcd_Write_String("   ");
                    };
                    Lcd_Set_Cursor(j+1,7);
                } else {
                    Lcd_Set_Cursor(j+1,10);
                };
                if (active[j]==1) {
                    draw_temp(temp[j]);
                } else {
                    Lcd_Write_String(" ---   ");
                };  
            };
        };
    } else {
            // более двух датчиков
            for (j=0;j<4;j++) {

                switch(j) {
                    case 0:
                        Lcd_Set_Cursor(1,3);    //  первый датчик
                        break;
                    case 1:
                        Lcd_Set_Cursor(1,11);    //  второй датчик
                        break;
                    case 2:
                        Lcd_Set_Cursor(2,3);    //  третий датчик
                        break;
                    case 3:
                        Lcd_Set_Cursor(2,11);    //  четвертый датчик
                        break;
                };

                if (active[j]==1) {
                    draw_temp(temp[j]);
                } else {
                    Lcd_Write_String(" ---  ");
                };
            };
    };
}

void main(void) {
    
    INTCON = 0b11000000;; //разрешить прерывания от периферии
    RCIE=1;    // разрешаем прерывание по приему байта UART
    CMCON = 0b111; //Disable PORTA Comparators
    
    
    TRISB = 0b00000010;
    RESET_WIFI_STATE = 0; // Переключаем на ВЫХОД ногу для ресета ВАЙ-ФАЙ
    RESET_WIFI_PIN = 0;   // начинаем ресет вайфая
    Lcd_Init();
    init_comms();   // старт UART
    
    Lcd_Clear();
    Lcd_Set_Cursor(1,1);
    Lcd_Write_String("WIFI-Thermometer");
    Lcd_Set_Cursor(2,1);
    Lcd_Write_String("is starting...");
    
    __delay_ms(1000);
    
    //print_to_uart("System start");
    
    Lcd_Clear();
    //draw_screen();

    RESET_WIFI_PIN = 1; // заканчиваем ресет вай-фая
    
    TRISAbits.TRISA2 = 0; // переключаем RA2 на выход
    
    
    while(1){
        // считываем датчики в цикле
        for (j=0; j<4; j++) {
            t = get_temp(j);
            if (nosensor==1) {
                //printf("temp%d = ---\r\n", j ); 
                active[j] = 0;      // датчик отключен от термометра
                
            } else {
                active[j] = 1;      // датчик подключен к термометру
                temp[j] = t;        // обновляем массив текущих температур
                
                if(temp[j]>tempmax[j]) tempmax[j]=temp[j];
                if(temp[j]<tempmin[j]) tempmin[j]=temp[j];
            };
        };
        RCIE = 0; //запрещаем прерывания
        draw_screen();
        RCIE = 1; //запрещаем прерывания
        __delay_ms(500);
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
                    if (a[2]==' ' && a[3]=='R' && a[4]=='\0') { // сбрасываем мин/мах значения
                        tempmin[0] = tempmin[1] = tempmin[2] = tempmin[3] = 999;      // минимальные показания
                        tempmax[0] = tempmax[1] = tempmax[2] = tempmax[3] = -999;  // максимальные показания
                        //temp[0] = temp[1] = temp[2] = temp[3] = 0;                  // текущее показания
                        //print_to_uart("Reseted\r\n");
                    };
                    if (a[2]==' ' && a[3]=='1' && a[4]=='\0') { //AT 1\0  - текущая температура
                        printf("%d|%d|%d|%d\r\n",temp[0],temp[1],temp[2],temp[3]);
                        //printf("%d\r\n",temp[0]);
                    };
                    if (a[2]==' ' && a[3]=='2' && a[4]=='\0') { //AT 1\0  - мин температура
                        printf("%d|%d|%d|%d\r\n",tempmin[0],tempmin[1],tempmin[2],tempmin[3]);
                        //printf("%d\r\n",tempmin[0]);
                    };
                    if (a[2]==' ' && a[3]=='3' && a[4]=='\0') { //AT 1\0  - макс температура
                        printf("%d|%d|%d|%d\r\n",tempmax[0],tempmax[1],tempmax[2],tempmax[3]);
                        //printf("%d\r\n",tempmax[0]);
                    };
                    if (a[2]==' ' && a[3]=='4' && a[4]=='\0') { //AT 1\0  - активные датчики
                        printf("%d|%d|%d|%d\r\n",active[0],active[1],active[2],active[3]);
                        //printf("%d\r\n",active[0]);
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