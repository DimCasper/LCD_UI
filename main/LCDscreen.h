#ifndef LCDSCREEN_H_INCLUDED
#define LCDSCREEN_H_INCLUDED

#include "config.h"
#include "general.h"
#include <stdint.h>
#include <arduino.h>
#include <avr/pgmspace.h>

#ifdef LCD_2004
    #define LCD_COLS 20
    #define LCD_ROWS 4
    #define LCD_SIZE 80
#elif defined(LCD_12864)
    #define LCD_COLS 16
    #define LCD_ROWS 4
    #define LCD_SIZE 64
#endif

#if defined(LCD_2004) || defined(LCD_1604) || defined(LCD_12864)
  #define LCD_CHOOSEN
#endif // LCD type

typedef int format_t;
#define ALIGN_L 0
#define ALIGN_R 1

typedef struct {
    char title[LCD_COLS+1];
    void* pointto;
    bool hasSubmenu;
    int16_t submenuLength;
}menuTable;

    void LCDScreenInit();

    void welcomeDisplay();
    //功能
    bool menuInit();
    bool menuInit(int _length,menuTable* table);
    bool menuInit(int _length,String (*_getLine)(int) = nullptr,void (*_enterCallback)(int) = nullptr,void (*endCallback)() = nullptr);
    void menuPrint(bool datamem = NORMALDATA);
    void menuDisplay(const char*,uint8_t,bool datamem = NORMALDATA);//內容，行，內容位置顯示
    void menuDisplay(String,uint8_t,bool datamem = NORMALDATA);
    void menuUp();
    void menuDown();
    void menuPageDown(String);
    void menuPageDown(const char*);
    void menuPageUp(String);
    void menuPageUp(const char*);
    void menuTableEnter();
    void menuEnter();
    void menuBack();
    void menuClear();
    String menuTableGetLine(int line);
    //箭頭移動
    void menuCursorMove(uint8_t,uint8_t);
    void menuCursorMove(uint8_t);
    void menuCursorDown();
    void menuCursorUp();
    uint8_t menuGetCusorPos();
    bool menuCusorAtBottom();
    bool menuCusorAtTop();
    //開機介面
    void statusDisplay();
    void progressBar(int8_t);
    void runtime(uint32_t);
    void customtime(uint32_t);
    void statusSet(uint8_t);

    void printline(const char *str,uint8_t line = 0,format_t format = ALIGN_L);
    void printline_P(const char *str,uint8_t line = 0);

    void flash();
    void clear();
    //未用到
    void printf(int value,int width,bool zero = false);
    // void printf(const char *str,int width,format_t format = ALIGN_L);


#define STATUS_IDLE 0
#define STATUS_RUN  1
#define STATUS_STOP 2

#if defined(LCD_12864)
    #define STATUS_ROW 2
    #define STATUS_IDLE_POS 0
    #define STATUS_RUN_POS  0
    #define STATUS_STOP_POS 0
    #define STATUS_LENGTH 5

    #define PROGRESSBAR_LENGTH 10

    #define TIME_ROW 1
    #define TIME_RUN 0
    #define TIME_CUS (8/2)
    #define TIME_HRLEN 1
    #define TIME_MNLEN 2
    #define TIME_SECLEN 2

const unsigned char statusString[16+1] PROGMEM = " IDLE RUN  STOP";
#else
    #define STATUS_ROW 2
    #define STATUS_IDLE_POS 13
    #define STATUS_RUN_POS  2
    #define STATUS_STOP_POS 7
    #define STATUS_LENGTH 6

    #define PROGRESSBAR_LENGTH 16

    #define TIME_ROW 1
    #define TIME_RUN 1
    #define TIME_CUS 11
    #define TIME_HRLEN 2
    #define TIME_MNLEN 2
    #define TIME_SECLEN 2

const unsigned char statusString[18+1] PROGMEM = " IDLE   RUN  STOP ";
#endif // 12864

#if defined(LCD_12864)
const unsigned char mainpage[LCD_SIZE] PROGMEM = {
    '0','0','<',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','>',
    '0',':','0','0',':','0','0',' ','0',':','0','0',':','0','0',' ',
    ' ',' ',' ',' ',' ',' ',' ',' ',' ','I','D','L','E',' ',' ',' ',
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '
};

const unsigned char welcomepage[LCD_SIZE] PROGMEM = {
    ' ',' ',0xE5,0xDF,0x56,0xDF,0x29,0xDF,0x56,0xDF,0x29,0xE5,' ',' ',' ',' ',
    ' ',' ',' ',0xE5,0xDF,0x56,0xDF,0x29,0xDF,0x56,0xDF,0x29,0xE5,' ',' ',' ',
    ' ',' ',' ',' ',' ',' ',' ','M','a','d','e',' ','B','y',' ',' ',
    'L','o','a','d','i','n','g','D','i','m','C','a','s','p','e','r'
};
#else
const unsigned char mainpage[LCD_SIZE] PROGMEM = {
    '0','0','<',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','>',
    ' ','0','0',':','0','0',':','0','0',' ',' ','0','0',':','0','0',':','0','0',' ',
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','I','D','L','E',' ',' ',
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '
};

const unsigned char welcomepage[LCD_SIZE] PROGMEM = {
    ' ',' ',0xE5,0xDF,0x56,0xDF,0x29,0xDF,0x56,0xDF,0x29,0xE5,' ',' ',' ',' ',' ',' ',' ',' ',
    ' ',' ',' ',0xE5,0xDF,0x56,0xDF,0x29,0xDF,0x56,0xDF,0x29,0xE5,' ',' ',' ',' ',' ',' ',' ',
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','M','a','d','e',' ','B','y',' ',' ',
    'L','o','a','d','i','n','g','.','.','.',' ','D','i','m','C','a','s','p','e','r'
};
#endif // 12864

#endif // LCDSCREEN_H_INCLUDED
