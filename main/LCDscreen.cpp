#include "LCDscreen.h"
#include <stdlib.h>
#include <string.h>
#include <arduino.h>

// #define DEBUG

char displaymem[LCD_SIZE];
uint8_t _menuCursor;

#ifndef LCD_CHOOSEN
uint8_t _cols;  // real number, greater than 1
uint8_t _rows;  // real number, greater than 1
#endif // LCD_CHOOSEN

#ifdef LCD_IIC
    #include "NewliquidCrystal/LiquidCrystal_I2C_ByVac.h"
    LiquidCrystal_I2C_ByVac lcdsc;
#elif defined(LCD_PARALLEL_4) || defined(LCD_PARALLEL_8)
    #include "LiquidCrystal.h"
    #if defined(LCD_PARALLEL_4)
        LiquidCrystal lcdsc(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
    #elif defined(LCD_PARALLEL_8)
        LiquidCrystal lcdsc(LCD_RS, LCD_EN, LCD_D0, LCD_D1, LCD_D2, LCD_D3, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
    #endif
#endif // LCD_LINE_SELECT

#ifdef LCD_CHOOSEN
    #define _cols LCD_COLS
    #define _rows LCD_ROWS
    void LCDScreenInit()
    {
        //displaymem = (char*)malloc((_cols*_rows+1)*sizeof(char));
        #if defined(LCD_12864)
        lcdsc.begin(_cols/2,_rows);
        #else
        lcdsc.begin(_cols,_rows);
        #endif // type of LCD
    }
#else
void LCDScreenInit(uint8_t cols = 16, uint8_t rows = 2) : _cols(cols),_rows(rows)
{
    //displaymem = (char*)malloc((cols*rows+1)*sizeof(char));
    lcdsc.begin(_cols,_rows);
}
#endif // LCD_CHOOSEN

void welcomeDisplay()
{
    #if defined(DEBUG)
        Serial.println(F("welcomeDisplay"));
    #endif // defined
    lcdsc.clear();
    delay(2);   // make sure lcd clear, it takes long time
    char tmp;
    for(int i=0;i<LCD_SIZE;i++)
    {
        #if LCD_ROWS>2
        if(i%_cols==0) lcdsc.setCursor(0,i/_cols);
        #endif // LCD_ROWS>2
        tmp = pgm_read_byte(welcomepage+i);
        lcdsc.write(tmp);
    }
}

/************************
 *                      *
 *     Menu function    *
 *                      *
 ************************/

void menuInit()
{
    lcdsc.clear();
    delay(2);   // make sure lcd clear, it takes long time
    _menuCursor = 0;
    for(int i=0;i<LCD_SIZE;i++)
    {
        displaymem[i] = ' ';
    }
}

void menuDisplay(const char* item, uint8_t row, uint8_t col)
{
    if(row>_rows) row=_rows;
    if(col>_cols) col=_cols;
    col -= 1;  // set to real column
    row -= 1;  // set to real row
    #if defined(LCD_12864)
    lcdsc.setCursor(col/2,row);
    if(col%2)
    {
        lcdsc.write(displaymem[row*_cols+col-1]);
    }
    #else
    lcdsc.setCursor(col,row);
    #endif // 12864
    for(;col<_cols;col++)
    {
        if(*item)
        {
            lcdsc.write(*item);
            (*(displaymem+row*_cols+col)) = (*item);
            item++;
        }
        else
        {
            lcdsc.write(' ');
            (*(displaymem+row*_cols+col)) = ' ';
        }
    }
}

void menuDisplay_P(const char* item, uint8_t row, uint8_t col)
{
    col -= 1;  // set to real column
    row -= 1;  // set to real row
    #if defined(LCD_12864)
    lcdsc.setCursor(col/2,row);
    if(col%2)
    {
        lcdsc.write(displaymem[row*_cols+col-1]);
    }
    #else
    lcdsc.setCursor(col,row);
    #endif // 12864
    for(;col<_cols;col++)
    {
        if(pgm_read_byte(item))
        {
            displaymem[row*_cols+col] = pgm_read_byte(item);
            lcdsc.write(pgm_read_byte(item));
            item++;
        }
        else
        {
            lcdsc.write(' ');
            displaymem[row*_cols+col] = ' ';
        }
    }
}

void menuCursorMove(uint8_t from,uint8_t to)
{
    if(from>=_rows) from = _rows;
    if(to>=_rows) to = _rows;
    if(from==0) from = 1;
    if(to==0) to = 1;
    _menuCursor = to;

    // set to real row in LCD
    from -= 1;
    to -= 1;

    #if defined(DEBUG)

    #endif // defined

    lcdsc.setCursor(0,from);
    lcdsc.write(' ');
    #if defined(LCD_12864)
    lcdsc.write(displaymem[from*_cols+1]);
    #endif // 12864
    displaymem[(from)*_cols] = ' ';
    lcdsc.setCursor(0,to);
    lcdsc.write('>');
    #if defined(LCD_12864)
    lcdsc.write(displaymem[to*_cols+1]);
    #endif // 12864
    displaymem[(to)*_cols] = '>';
}

void menuCursorMove(uint8_t to)
{
    menuCursorMove(_menuCursor ,to);
}

void menuCursorDown()
{
    if(_menuCursor == _rows) return ;
    menuCursorMove(_menuCursor,_menuCursor+1);
}

void menuCursorUp()
{
    if(_menuCursor == 1) return ;
    menuCursorMove(_menuCursor,_menuCursor-1);
}

void menuDown(const char *item)
{
    for(int j=2;j<=_rows;j++)
    {
        menuDisplay(displaymem+1+_cols*(j-1),j-1);
    }
    menuDisplay(item,_rows);
}

void menuUp(const char *item)
{
    for(int j=_rows-1;j>=1;j--)
    {
        menuDisplay(displaymem+1+_cols*(j-1),j+1);
    }
    menuDisplay(item,1);
}

/************************
 *                      *
 *   Status function    *
 *                      *
 ************************/

void statusDisplay()
{
    #if defined(DEBUG)
    // Serial.println(F("statusDisplay"));
    #endif // DEBUG
    lcdsc.clear();
    delay(2);   // make sure lcd clear, it takes long time
    char tmp;
    for(int i=0;i<LCD_SIZE;i++)
    {
        #if LCD_ROWS>2
        if(i%_cols==0) lcdsc.setCursor(0,i/_cols);
        #endif // LCD_ROWS>2
        tmp = pgm_read_byte(mainpage+i);
        //displaymem[i] = tmp;
        lcdsc.write(tmp);
    }
    runtime(millis());
    customtime(millis());
    statusSet(STATUS_IDLE);

}

#define STATUS_SET(STATUS) \
    if(status_code == STATUS) \
    { \
        for(uint8_t i=0;i<STATUS_LENGTH;i++) \
        { \
            tmp = pgm_read_byte(statusString+STATUS*STATUS_LENGTH+i); \
            displaymem[_cols*2+2+STATUS*6+i] = tmp; \
            lcdsc.write(tmp); \
        } \
    } \
    else \
    { \
        for(uint8_t i=0;i<STATUS_LENGTH;i++) \
        { \
            displaymem[_cols*2+2+STATUS*STATUS_LENGTH+i] = ' '; \
            lcdsc.write(' '); \
        } \
    }

void statusSet(uint8_t status_code = STATUS_IDLE)
{
    char tmp;
    lcdsc.setCursor(STATUS_IDLE_POS,STATUS_ROW);
    STATUS_SET(STATUS_IDLE);
    STATUS_SET(STATUS_RUN);
    STATUS_SET(STATUS_STOP);
}

void progressBar(int8_t progress)
{
    lcdsc.home();
    if(progress>=100||progress<0) progress = 0;
    printf(progress,2,true);
    lcdsc.moveCursorRight();
    for(uint8_t i=0;i<PROGRESSBAR_LENGTH;i++)
    {
        if(progress-100/(PROGRESSBAR_LENGTH+1)>0)
        {
            #if defined(LCD_12864)
            lcdsc.write(0x08);
            #else
            lcdsc.write(0xFF);
            #endif // 12864
            progress-=100/(PROGRESSBAR_LENGTH+1);
        }
        else lcdsc.write(' ');
    }
}

void runtime(uint32_t _time)
{
    _time/=1000UL;
    uint8_t sec = _time%60UL;
    uint8_t mn  = (_time%3600UL)/60UL;
    uint8_t hr  = _time/3600UL;
    lcdsc.setCursor(TIME_RUN,TIME_ROW);
    printf(hr ,TIME_HRLEN,true);
    // lcdsc.moveCursorRight();
    lcdsc.write(':'); // make sure can run in 12864
    printf(mn ,TIME_MNLEN,true);
    // lcdsc.moveCursorRight();
    lcdsc.write(':'); // make sure can run in 12864
    printf(sec,TIME_SECLEN,true);
}

void customtime(uint32_t _time)
{
    _time/=1000UL;
    uint8_t sec = _time%60;
    uint8_t mn  = (_time%3600)/60;
    uint8_t hr  = _time/3600;
    lcdsc.setCursor(TIME_CUS,TIME_ROW);
    printf(hr ,TIME_HRLEN,true);
    // lcdsc.moveCursorRight();
    lcdsc.write(':'); // make sure can run in 12864
    printf(mn ,TIME_MNLEN,true);
    // lcdsc.moveCursorRight();
    lcdsc.write(':'); // make sure can run in 12864
    printf(sec,TIME_SECLEN,true);
}

/************************
 *                      *
 *        Other         *
 *                      *
 ************************/

void printf(int value,int width,bool zero)
{
    int ten = 1,&tmp=width;
    for(;width>1;width--)
    {
        ten *= 10;
    }
    for(;ten!=0;ten/=10)
    {
        tmp = value/ten;
        if(tmp == 0)
        {
            if(zero) lcdsc.write('0');
            else     lcdsc.write(' ');
        }
        else
        {
            tmp%=10;
            lcdsc.write(tmp+'0');
        }
    }
}

void printline(const char *str,uint8_t line,format_t format)
{
    lcdsc.setCursor(0,line-1);
    uint8_t i=0,j=0;
    if(format == ALIGN_R)
    {
        int space = strlen(str)-_cols;
        for(;space>0;space--,i++)
        {
            lcdsc.write(' ');
        }
        if(space<0)
        {
            j=abs(space)>strlen(str)?strlen(str):abs(space);
        }
    }
    for(;i<_cols;i++)
    {
        if(str[j]!='\0')
        {
            lcdsc.write(str[j]);
            j++;
        }
        else
        {
            lcdsc.write(' ');
        }
    }
}

void printline_P(const char *str,uint8_t line)
{
    lcdsc.setCursor(0,line-1);
    uint8_t i=0;
    for(;i<_cols;i++)
    {
        if(pgm_read_byte(str)!='\0')
        {
            lcdsc.write(pgm_read_byte(str));
            str++;
        }
        else
        {
            lcdsc.write(' ');
        }
    }
}

void flash()
{
    lcdsc.noDisplay();
    delay(100);
    lcdsc.display();
}

void clear()
{
    lcdsc.clear();
    delay(2);   // make sure lcd clear, it takes long time
}
