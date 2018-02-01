#include "LCDscreen.h"
#include "mystack.h"
#include <stdlib.h>
#include <string.h>
#include <arduino.h>

// #define DEBUG

char displaymem[LCD_SIZE];

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
        lcdsc.setRowOffsets(0x80, 0x90, 0x88, 0x98);
        #else
        lcdsc.begin(_cols,_rows);
        #endif // type of LCD
    }
#else

void LCDScreenInit(uint8_t cols = 16, uint8_t rows = 2) : _cols(cols),_rows(rows)
{
    //displaymem = (char*)malloc((cols*rows+1)*sizeof(char));
    lcdsc.begin(_cols,_rows);
    #if defined(LCD_12864)
    lcdsc.setRowOffsets(0x80, 0x90, 0x88, 0x98);
    #endif // defined
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

String (*getLine)(int) = nullptr;
void (*menuEnterCallback)(int) = nullptr;
void (*menuEndCallback)() = nullptr;

typedef struct {
    int8_t item_head;
    int8_t item_cursor;
    int8_t item_length;
    void *menuptr;
}menu_stack_item;

mystack<menu_stack_item> menu_stack;

void menuPrint(bool datamem = NORMALDATA)
{
    clear();
    menuCursorMove(menu_stack.end()->item_cursor);
    for(int row=0;row<_rows&&row+menu_stack.end()->item_head<menu_stack.end()->item_length;row++)
    {
        #ifdef DEBUG
        Serial.print(F("Row:"));
        Serial.print(row);
        Serial.print(F("\tText:"));
        Serial.println(getLine(row));
        #endif // DEBUG
        menuDisplay(getLine(row+menu_stack.end()->item_head),row,datamem);
    }
}

void menuPrint(String (*_getLine)(int))
{
    getLine = _getLine;
    menuPrint();
}

bool menuInit()
{
    lcdsc.clear();
    delay(2);   // make sure lcd clear, it takes long time
    menu_stack.add({0,0,0,nullptr});
    menuCursorMove(0);
    for(int i=0;i<LCD_SIZE;i++)
    {
        displaymem[i] = ' ';
    }
    return true;
}

bool menuInit(int _length,String (*_getLine)(int),void (*_enterCallback)(int),void (*_endCallback)())
{
    menuInit();
    menu_stack.end()->item_length = _length;
    if(_getLine) getLine = _getLine;
    if(_enterCallback) menuEnterCallback = _enterCallback;
    if(_endCallback) menuEndCallback = _endCallback;
    menuPrint();
    return true;
}

bool menuInit(int _length,menuTable* table)
{
    if(!table)
    {
        return false;
    }
    menuInit();
    menu_stack.end()->item_length = _length;
    getLine = menuTableGetLine;
    menuEnterCallback = menuTableEnter;

    menu_stack.add({0,0,_length,table});
    menuPrint();
    return true;
}

void menuDisplay(String text,uint8_t row,bool datamem)
{
    menuDisplay(text.c_str(),row,datamem);
}

void menuDisplay(const char* item, uint8_t row, bool datamem)
{
    if(row>_rows) row=_rows;
    int col=2;
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
        char temp = datamem?pgm_read_byte(item):*item;
        if(temp)
        {
            lcdsc.write(temp);
            displaymem[row*_cols+col] = (temp);
            item++;
        }
        else
        {
            lcdsc.write(' ');
            displaymem[row*_cols+col] = ' ';
        }
    }
}

String menuTableGetLine(int line)
{
    menuTable tempItem;
    memcpy_P(&tempItem, (menu_stack.end()->menuptr)+(line)*sizeof(menuTable),sizeof(menuTable));
    return String((tempItem.title));// reinterpret_cast<const __FlashStringHelper *>
}

void menuTableEnter()
{
    menuTable tempItem;
    memcpy_P(&tempItem, ((menu_stack.end()->menuptr)+(menu_stack.end()->item_head+menu_stack.end()->item_cursor)*sizeof(menuTable)),sizeof(menuTable));
    if(!(tempItem.hasSubmenu||tempItem.pointto))
    {
        // back to previous menu
        menuBack();
    }
    else
    {
        // enter select
        if(tempItem.hasSubmenu)
        {
            menu_stack.add({0,0,tempItem.submenuLength,tempItem.pointto});

            menuPrint();
        }
        else
        {
            ((void (*)())(tempItem.pointto))();
        }
    }
}

void menuUp()
{
    #ifdef DEBUG
    Serial.println(F("menuUP"));
    #endif // DEBUG
    if(!(menu_stack.end()->item_cursor+menu_stack.end()->item_head))
    {
        flash();
        return ;
    }
    if(!menu_stack.end()->item_cursor)
    {
        (menu_stack.end()->item_head)--;
        menuPageUp(getLine(menu_stack.end()->item_head));
        return;
    }
    menuCursorUp();
}

void menuDown()
{
    #ifdef DEBUG
    Serial.println(F("menuDown"));
    #endif // DEBUG
    if(menu_stack.end()->item_cursor+menu_stack.end()->item_head+1==menu_stack.end()->item_length)
    {
        flash();
        return ;
    }
    if(menu_stack.end()->item_cursor+1==_rows)
    {
        (menu_stack.end()->item_head)++;
        menuPageDown(getLine(menu_stack.end()->item_head+menu_stack.end()->item_cursor));
        return;
    }
    menuCursorDown();
}

void menuEnter()
{
    menuEnterCallback(menu_stack.end()->item_cursor+menu_stack.end()->item_head);
}

void menuBack()
{
    menu_stack.pop_back();
    if(menu_stack.size())
    {
        menuPrint();
    }
    else
    {
        if(!menuEndCallback) menuEndCallback();
    }
}

void menuClear()
{
    menu_stack.clear();
}

void menuCursorMove(uint8_t from,uint8_t to)
{
    if(from>=_rows) from = _rows;
    if(to>=_rows) to = _rows;
    menu_stack.end()->item_cursor = to;

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
    menuCursorMove(menu_stack.end()->item_cursor ,to);
}

void menuCursorDown()
{
    if(menu_stack.end()->item_cursor == _rows-1) return ;
    menuCursorMove(menu_stack.end()->item_cursor,menu_stack.end()->item_cursor+1);
}

void menuCursorUp()
{
    if(menu_stack.end()->item_cursor == 0) return ;
    menuCursorMove(menu_stack.end()->item_cursor,menu_stack.end()->item_cursor-1);
}

void menuPageDown(String item)
{
    menuPageDown(item.c_str());
}

void menuPageDown(const char *item)
{
    for(int j=1;j<_rows;j++)
    {
        menuDisplay(displaymem+2+_cols*j,j-1);
    }
    menuDisplay(item,_rows-1);
}

void menuPageUp(String item)
{
    menuPageUp(item.c_str());
}

void menuPageUp(const char *item)
{
    for(int j=_rows-1-1;j>=0;j--)
    {
        menuDisplay(displaymem+2+_cols*j,j+1);
    }
    menuDisplay(item,0);
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

}

void statSet(char* stat)
{
    lcdsc.setCursor(STAT_POS,STAT_ROW);
    for(int i=0;i<STAT_LENGTH&&(*stat);i++,stat++)
    {
        lcdsc.write(*stat);
    }
}

void statSet(String stat)
{
    statSet(stat.c_str());
}

void statSet(const status *s)
{
    statSet(s->stat);
    #if defined(LCD_12864)
    lcdsc.setCursor((STAT_X_POS+1)/2,STAT_ROW);
    if((STAT_X_POS+1)%2)
    {
        lcdsc.write(displaymem[STAT_ROW*_cols+(STAT_X_POS+1)-1]);
    }
    #else
    lcdsc.setCursor((STAT_X_POS+1),STAT_ROW);
    #endif // 12864
    printf(s->x,STAT_POS_LENGTH);
    #if defined(LCD_12864)
    lcdsc.setCursor((STAT_Y_POS+1)/2,STAT_ROW);
    if((STAT_Y_POS+1)%2)
    {
        lcdsc.write(displaymem[STAT_ROW*_cols+(STAT_Y_POS+1)-1]);
    }
    #else
    lcdsc.setCursor((STAT_Y_POS+1),STAT_ROW);
    #endif // 12864
    printf(s->y,STAT_POS_LENGTH);
    #if defined(LCD_12864)
    lcdsc.setCursor((STAT_Z_POS+1)/2,STAT_ROW);
    if((STAT_Z_POS+1)%2)
    {
        lcdsc.write(displaymem[STAT_ROW*_cols+(STAT_Z_POS+1)-1]);
    }
    #else
    lcdsc.setCursor((STAT_Z_POS+1),STAT_ROW);
    #endif // 12864
    printf(s->z,STAT_POS_LENGTH);
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
    lcdsc.setCursor(0,line);
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
    lcdsc.setCursor(0,line);
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
