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


#if defined(LCD_12864)
    #define STAT_ROW 2
    #define STAT_POS 0
    #define STAT_LENGTH 5
    #define STAT_X_POS (STAT_LENGTH+3*0)
    #define STAT_Y_POS (STAT_LENGTH+3*1)
    #define STAT_Z_POS (STAT_LENGTH+3*2)
    #define STAT_POS_LENGTH 2

    #define PROGRESSBAR_LENGTH 10

    #define TIME_ROW 1
    #define TIME_RUN 0
    #define TIME_CUS (8/2)
    #define TIME_HRLEN 1
    #define TIME_MNLEN 2
    #define TIME_SECLEN 2

#else
    #define STAT_ROW 2
    #define STAT_POS 0
    #define STAT_LENGTH 5
    #define STAT_X_POS (STAT_LENGTH+5*0)
    #define STAT_Y_POS (STAT_LENGTH+5*1)
    #define STAT_Z_POS (STAT_LENGTH+5*2)
    #define STAT_POS_LENGTH 2


    #define PROGRESSBAR_LENGTH 16

    #define TIME_ROW 1
    #define TIME_RUN 1
    #define TIME_CUS 11
    #define TIME_HRLEN 2
    #define TIME_MNLEN 2
    #define TIME_SECLEN 2

#endif // 12864

#if defined(LCD_12864)
const unsigned char mainpage[LCD_SIZE] PROGMEM = {
    '0','0','<',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','>',
    '0',':','0','0',':','0','0',' ','0',':','0','0',':','0','0',' ',
    ' ',' ',' ',' ',' ','X',' ',' ',' ','Y',' ',' ',' ','Z',' ',' ',
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
    ' ',' ',' ',' ',' ','X',' ',' ',' ',' ','Y',' ',' ',' ',' ','Z',' ',' ',' ',' ',
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '
};

const unsigned char welcomepage[LCD_SIZE] PROGMEM = {
    ' ',' ',0xE5,0xDF,0x56,0xDF,0x29,0xDF,0x56,0xDF,0x29,0xE5,' ',' ',' ',' ',' ',' ',' ',' ',
    ' ',' ',' ',0xE5,0xDF,0x56,0xDF,0x29,0xDF,0x56,0xDF,0x29,0xE5,' ',' ',' ',' ',' ',' ',' ',
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','M','a','d','e',' ','B','y',' ',' ',
    'L','o','a','d','i','n','g','.','.','.',' ','D','i','m','C','a','s','p','e','r'
};
#endif // 12864

typedef struct status
{
    char stat[STAT_LENGTH+1];
    int8_t x;
    int8_t y;
    int8_t z;
};

typedef int format_t;
#define ALIGN_L 0
#define ALIGN_R 1

typedef struct {
    char title[16+1];
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
    void menuPrint(String (*_getLine)(int));
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
    void statSet(String);
    void ststSet(char*);
    void statSet(const status*);

    void printline(const char *str,uint8_t line = 0,format_t format = ALIGN_L);
    void printline_P(const char *str,uint8_t line = 0);

    void flash();
    void clear();
    //未用到
    void printf(int value,int width,bool zero = false);
    // void printf(const char *str,int width,format_t format = ALIGN_L);

#endif // LCDSCREEN_H_INCLUDED
