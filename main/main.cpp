#include <arduino.h>
#include <avr/pgmspace.h>
#include "keyDetect.h"
#include "mylist.h"
#include "LCDscreen.h"
#include "config.h"
#include "stringlist.h"

// #include <FreeStack.h>

/**
 *  If you don't have this library, please download it at
 *  https://github.com/greiman/SdFat
 **/
#include <SdFat.h>

keyDetect key;

SdFat SD;
SdBaseFile runfile;
mylist<uint16_t> fileList;
stringlist path;

#define EOF -1

/** ===================================== **/

// act
void act();
void printmenulist();
void homing ();
void SDselect ();
void unlock ();

// key callback
void upCallback();
void downCallback();
void enterCallback();

void getfilelist();
void printfilelist();
inline void path_popback();
// inline void path_back();

// main control
bool sendLine();
int fileCompare(const void* a,const void* b);

// other
bool equalsWithPgmString(const char* , const char*);

/** ===================================== **/

uint32_t pretime_status_screen = 0;
uint32_t run_start_time = 0;

/** ===================================== **/

// flag use as an integer
uint8_t flag_screen=0;
enum screen
{
    STATUSPAGE,
    MENUPAGE,
    FILEPAGE,
    SDERRORPAGE
};

// flag use bit set
uint8_t flag_controlState=0;
enum controlState
{
    STANDBY,
    RUNNING,
    WAITING_RESPONSE
};

/**  **/

// typedef bool submenu_t;
//子目錄
typedef struct {
    char title[LCD_COLS+1];
    void* pointto;
    bool hasSubmenu;
}menulist;
//目錄
PROGMEM const menulist menu_0[] = {
{"Status page"          , (void*)NULL     , false},
{"Homing Setting"       , (void*)homing   , false},
{"Open SD card"         , (void*)SDselect , false},
{"Unlock"               , (void*)unlock   , false}
};

typedef struct {
    uint8_t item_head;
    uint8_t item_select;
    void *menuptr;
}menu_item;

mylist<menu_item> menu_stack;

/** ============ arduino main ============ **/

void setup()
{
    // set grbl baud rate
    Serial.begin(BAUD_RATE);
    Serial.setTimeout(SERIAL_TIMEOUT);

    // print init to lcd
    LCDScreenInit();
    welcomeDisplay();

    // set callback function
    key.setCallback(KEY_DOWN, downCallback);
    key.setCallback(KEY_UP, upCallback);
    key.setCallback(KEY_ENTER, enterCallback);

    // start
    delay(1000UL); // pause welcome scene at least 0.5s
    statusDisplay();
}

void loop ()
{
    act();

    key.detect();
}

bool readOK(String& t)
{
  return equalsWithPgmString(t.c_str(),PSTR("ok"));
  /*
  for(int i=0;i<t.length();i++)
    if(t.charAt(i)=='o' && t.charAt(i+1) == 'k')
      return 1;
  return 0;
  */
}

void serialEvent()
{
    if(Serial.available()<2) return;
    #if defined(DEBUG)
    String temp;
    for(uint32_t timeout=millis() ; millis() - timeout<SERIAL_TIMEOUT;)
    {
        if(Serial.available())
        {
            char c = Serial.read();
            if(c=='\r' || c=='\n') break;
            temp+=c;
            delay(10);
        }
    }
    #else // NOT_DEBUG
    for(;Serial.peek()=='\r'||Serial.peek()=='\n';) Serial.read();
    String temp;
    //讀取資料
    for(uint32_t timeout=millis() ; millis() - timeout<SERIAL_TIMEOUT;)
    {
        if(Serial.available())
        {
            timeout=millis();
            char c = Serial.read();
            if(c=='\r' || c=='\n') break;
            temp+=c;
            delay(10);
        }
    }
    for(;Serial.peek()=='\r'||Serial.peek()=='\n';) Serial.read();
    for(;Serial.available();Serial.read());
    /*
    String temp = Serial.readStringUntil('\r');
    Serial.readStringUntil('\n'); // grbl response end by \r\n
    */
    #endif // if DEBUG else
    //讀旗標
    if(bitRead(flag_controlState,RUNNING) && bitRead(flag_controlState,WAITING_RESPONSE))
    {
        //delay(1000);
        if(readOK(temp))//讀OK
        {
            bitClear(flag_controlState,WAITING_RESPONSE);
            delay(10);
        }
        else
        {
            flag_screen = STATUSPAGE;
            statusDisplay();
            printline(temp.c_str(),4);
            runfile.close();
            #if defined(DEBUG)
            Serial.println(F("ERROR."));//讀ERROR(需更改
            Serial.println(temp);
            #endif // define
            bitSet(flag_controlState,STANDBY);
            bitClear(flag_controlState,RUNNING);
            bitClear(flag_controlState,WAITING_RESPONSE);
            delay(10);
        }
    }
    else
    {
        //
    }
    #if defined(DEBUG)
    for(;Serial.peek()=='\r'||Serial.peek()=='\n';) Serial.read();
    #endif // DEBUG
}

/** ======= Callback function ======= **/

uint8_t item_select = 0;
uint8_t item_head = 0;
uint8_t item_length = 0;

#if defined(DEBUG)
#define PRINT_ITEM_STATE \
    Serial.println((int)(item_head+item_select));
#define PRINT_ITEM \
    Serial.println(temp);
#else
#define PRINT_ITEM_STATE
#define PRINT_ITEM
#endif //DEBUG

#define MENU_READ_U \
            menulist tempItem; \
            memcpy_P(&tempItem, (menu_stack.end()->menuptr)+(item_head)*sizeof(menulist),sizeof(menulist)); \
            char *temp = tempItem.title;

#define MENU_READ_D \
            menulist tempItem; \
            memcpy_P(&tempItem, (menu_stack.end()->menuptr)+(item_head+item_select)*sizeof(menulist),sizeof(menulist)); \
            char *temp = tempItem.title;

#define FILE_READ_U \
            char temp[NAME_MAX_LENGTH]; \
            if(item_head==0) strcpy_P(temp, PSTR("../")); \
            else \
            { \
                SdBaseFile basefile; \
                basefile.open(SD.vwd(),fileList[item_head - 1],O_READ); \
                basefile.getName(temp,NAME_MAX_LENGTH-2); \
                if(basefile.isDir()) strcat_P(temp,PSTR("/")); \
                basefile.close(); \
            }

#define FILE_READ_D \
            char temp[NAME_MAX_LENGTH]; \
            SdBaseFile basefile; \
            basefile.open(SD.vwd(),fileList[item_head + item_select -1],O_READ); \
            basefile.getName(temp,NAME_MAX_LENGTH-1); \
            if(basefile.isDir()) strcat_P(temp,PSTR("/")); \
            basefile.close();

// MOVECHECK(MENU_READ,DOWN)
#define MOVECHECK(READTYPE,IF,IF2,DIR,DIR_OPR) \
            if(IF) \
            { \
                flash(); \
                return ; \
            } \
            else \
            { \
                if(IF2) \
                { \
                    item_head DIR_OPR; \
                    (menu_stack.end()->item_head) DIR_OPR; \
                    READTYPE; \
                    menu ## DIR (temp); \
                    PRINT_ITEM_STATE; \
                    PRINT_ITEM; \
                    return ; \
                } \
                else \
                { \
                    item_select DIR_OPR; \
                    (menu_stack.end()->item_select) DIR_OPR; \
                    menuCursor ## DIR (); \
                    PRINT_ITEM_STATE; \
                    return ; \
                } \
            }

void downCallback()
{
    #if defined(DEBUG)
    Serial.println(F("Down"));
    #endif // DEBUG
    if(flag_screen == STATUSPAGE)
    {
        // do nothing
    }
    else if(flag_screen == MENUPAGE)
    {
        MOVECHECK(MENU_READ_D,item_select+item_head+1 == item_length,item_select+1 == LCD_ROWS,Down,++);
    }
    else if(flag_screen == FILEPAGE)
    {
        MOVECHECK(FILE_READ_D,item_select+item_head == item_length,item_select+1 == LCD_ROWS,Down,++);
    }
    else
    {
        ;
    }
}

void upCallback()
{
    #if defined(DEBUG)
    Serial.println(F("Up"));
    #endif // DEBUG
    if(flag_screen == STATUSPAGE)
    {
        // do nothing
    }
    else if(flag_screen == MENUPAGE)
    {
        MOVECHECK(MENU_READ_U,item_select+item_head==0,item_select == 0,Up,--);
    }
    else if(flag_screen == FILEPAGE)
    {
        MOVECHECK(FILE_READ_U,item_select+item_head==0,item_select == 0,Up,--);
    }
    else
    {
        ;
    }
}

#undef MENU_READ
#undef FILE_READ
#undef MOVECHECK

void enterCallback()
{
    #if defined(DEBUG)
    Serial.println(F("Enter"));
    #endif // DEBUG
    if(flag_screen == STATUSPAGE)
    {
        flag_screen = MENUPAGE;

        //will do it at printmenulist()
        // item_select = 0;
        // item_head = 0;
        // item_length = 0;

        menu_stack.add(menu_item{0,0,(void*)&menu_0});
        menuInit();

        // print menu list
        printmenulist();
    }
    else if(flag_screen == MENUPAGE)
    {
        if(item_head==0&&item_select==0)
        {
            // back to previous menu
            menu_stack.pop_back();
            if(menu_stack.size()==0)
            {
                flag_screen = STATUSPAGE;
                statusDisplay();
            }
            else
            {
                printmenulist();
            }
        }
        else
        {
            // enter select
            menulist tempItem;
            memcpy_P(&tempItem, ((menu_stack.end()->menuptr)+(item_head+item_select)*sizeof(menulist)),sizeof(menulist));
            if(tempItem.hasSubmenu)
            {
                menu_stack.add({0,0,tempItem.pointto});

                printmenulist();
            }
            else
            {
                ((void (*)())(tempItem.pointto))();
            }
        }
    }
    else if(flag_screen == FILEPAGE)
    {
        // enter selected directory or file.
        if(item_head==0&&item_select==0)
        {
            menuInit();
            menu_stack.pop_back();
            if(path.size()==0)
            {
                flag_screen = MENUPAGE;
                printmenulist();
            }
            else
            {
                path_popback();
                getfilelist();
                printfilelist();
            }
        }
        else
        {
            SdBaseFile basefile;
            basefile.open(SD.vwd(),fileList[item_head+item_select-1],O_READ);
            if(basefile.isDir())
            {
                char temp[NAME_MAX_LENGTH];
                basefile.getName(temp,NAME_MAX_LENGTH-1);
                basefile.close();
                path.add(temp);
                menu_stack.add({0,0,(void*)NULL});
                SD.chdir(temp,true);
                getfilelist();
                printfilelist();
            }
            else // is file
            {
                if(bitRead(flag_controlState,RUNNING))
                {
                    return ;
                }

                // clear the path first, it might waste space
                path.clear();

                // clear menu stack
                menu_stack.clear();

                runfile = basefile;
                #if defined(DEBUG)
                /*
                Serial.print(F("Select file:"));
                char temp[NAME_MAX_LENGTH];
                runfile.getName(temp,NAME_MAX_LENGTH-1);
                Serial.println(temp);
                */
                #endif // defined
                // set control state
                flag_controlState = 0;
                bitSet(flag_controlState,RUNNING);
                // transfer screen
                flag_screen = STATUSPAGE;
                statusDisplay();

                // initial timer
                run_start_time = millis();
            }
        }
    }
    else if(flag_screen == SDERRORPAGE)
    {
        //
        flag_screen = MENUPAGE;
        printmenulist();
    }
    else
    {
        // what if else?
    }
}

/** Action Function **/

void act ()
{
    if(bitRead(flag_controlState,RUNNING))
    {
        if(!bitRead(flag_controlState,WAITING_RESPONSE))
        {
            sendLine();
        }
    }
    if((flag_screen==STATUSPAGE) && millis()-pretime_status_screen>STATUS_SCREEN_CYCLE)
    {
        pretime_status_screen = millis();
        runtime(millis());
        if(bitRead(flag_controlState,RUNNING))
        {
            statusSet(STATUS_RUN);
            customtime(millis()-run_start_time);
            progressBar(runfile.curPosition()*100/runfile.fileSize());
        }
        else
        {
            customtime(0);
            statusSet(STATUS_IDLE);
        }
    }
}

void homing ()
{
    if(bitRead(flag_controlState,WAITING_RESPONSE)||bitRead(flag_controlState,RUNNING)) return;
    Serial.println(F("$H"));
    bitSet(flag_controlState, WAITING_RESPONSE);
}
void unlock ()
{
    if(bitRead(flag_controlState,WAITING_RESPONSE)||bitRead(flag_controlState,RUNNING)) return;
    Serial.println(F("$X"));
    bitSet(flag_controlState, WAITING_RESPONSE);
}
void SDselect ()
{
    flag_screen = FILEPAGE;
    if(!SD.begin())
    {
        #if defined(DEBUG)
        Serial.println(F("SD begin error."));
        #endif // defined
        clear();
        printline_P(PSTR("No SD card."),1);
        flag_screen = SDERRORPAGE;
        return ;
    }
    menu_stack.add({0,0,(void*)NULL});
    // clear path
    path.clear();
    getfilelist();
    printfilelist();
}

inline void path_popback()
{
    path.pop_back();
    SD.chdir(); // change directory to root
    // enter to the last directory
    for(size_t i=0 ; i<path.size() ; i++)
    {
        SD.chdir(path[i]);
    }
}

void getfilelist()
{
    SdBaseFile basefile;
    fileList.clear();
    for(;basefile.openNext(SD.vwd(),O_READ);)
    {
    if(!basefile.isHidden() && !basefile.isSystem())
    {
    fileList.add(basefile.dirIndex());
    }
    basefile.close();
    }
    fileList.sort(fileCompare);
    item_length = fileList.size();
    #if defined(DEBUG)
    Serial.println((int)item_length);
    #endif // defined
}

/** --- **/
//讀檔案(行讀取)讀一個位元 EOF=end of file
inline void filereadline(FatFile& readfile,String& buffer)
{
    for(int temp=readfile.read() ; temp!=EOF && (temp!='\r' && temp!='\n') ; temp=readfile.read())
    {
        buffer += (char)temp;
        delay(5);
    }
    // read until no \r or \n
    for(int temp=readfile.peek() ; temp!=EOF&&(temp=='\r' || temp=='\n') ; temp=readfile.peek()) readfile.read();
}
//讀檔案
bool sendLine()
{
    String buffer;
    for(;runfile.peek()!=EOF && buffer.length()==0;)
    {
        filereadline(runfile,buffer);
        buffer.remove(buffer.indexOf(';'));
    }
    if(buffer.length()==0)
    {
        #if defined(DEBUG)
        Serial.println(F("Done!"));
        #endif // DEBUG
        printline_P(PSTR("Done!"),4);
        bitClear(flag_controlState,RUNNING);
        bitSet(flag_controlState,STANDBY);
        return false;
    }
    Serial.println(buffer);
    bitSet(flag_controlState,WAITING_RESPONSE);
    return true;
}

int fileCompareName(const void* a,const void* b)
{
  char name1[NAME_MAX_LENGTH],name2[NAME_MAX_LENGTH];
  ((SdBaseFile*)a)->getName(name1,NAME_MAX_LENGTH-1);
  ((SdBaseFile*)b)->getName(name2,NAME_MAX_LENGTH-1);
  return strcmp(name1,name2);
}

int fileCompare(const void* a,const void* b)
{
    SdBaseFile a_file,b_file;
    a_file.open(SD.vwd(),*((uint16_t*)a),O_READ);
    b_file.open(SD.vwd(),*((uint16_t*)b),O_READ);
  if(a_file.isDir())
  {
    if(b_file.isDir())
    {
      return fileCompareName(&a_file,&b_file);
    }
    return -1;
  }
  if(b_file.isDir())
  {
    if(((SdBaseFile*)a)->isDir())
    {
      return fileCompareName(&a_file,&b_file);
    }
    return 1;
  }
  dir_t _a,_b;
  a_file.dirEntry(&_a);
  b_file.dirEntry(&_b);
    if(_a.lastWriteDate==_b.lastWriteDate)
  {
    if(_a.lastWriteTime < _b.lastWriteTime) {return  1;}
    if(_a.lastWriteTime > _b.lastWriteTime) {return -1;}
    return fileCompareName(&a_file,&b_file);
  }
    else if (_a.lastWriteDate > _b.lastWriteDate) {return -1;}
    return 1;
}

/** other **/

// use to print first n items
void printmenulist()
{
    item_select = menu_stack.end()->item_select;
    item_head = menu_stack.end()->item_head;
    item_length = 0;
    menuCursorMove(item_select+1);
    // print menu list
    for(;;item_length++)
    {
        menulist tempItem;
        memcpy_P(&tempItem, (menu_stack.end()->menuptr+(item_length)*sizeof(menulist)),sizeof(menulist));
        if(tempItem.title[0]=='\0') break;
        if(item_length<LCD_ROWS)
        {
            menuDisplay(tempItem.title,item_length+1);

            #if defined(DEBUG)
            Serial.println(tempItem.title);
            #endif // DEBUG
        }
    }
}

// use to print first n items
void printfilelist()
{
    item_select = menu_stack.end()->item_select;
    item_head = menu_stack.end()->item_head;
    menuCursorMove(item_select+1);
    int i;
    // first item is back to previous directory
    if(!item_head)
    {
        menuDisplay_P(PSTR("../"),1);
        i=2;
    }
    else
    {
        i=1;
    }
    #if defined(DEBUG)
    Serial.println(F("../"));
    #endif // defined
    for(;i<=LCD_ROWS;i++)
    {
        char temp[NAME_MAX_LENGTH];
        SdBaseFile basefile;
        basefile.open(SD.vwd(),fileList[i-2+item_head],O_READ);
        basefile.getName(temp,NAME_MAX_LENGTH-2);
        if(basefile.isDir()) strcat_P(temp,PSTR("/"));
        menuDisplay(temp,i);
        basefile.close();

        #if defined(DEBUG)
        Serial.println(temp);
        #endif // DEBUG
    }
}

bool equalsWithPgmString(const char* a, const char* s)
{
    char temp;
    for(; (temp=pgm_read_byte(s++))&&(*a) ; a++)
    {
        if((*a) != temp) return false;
    }
    if((*a) != temp) return false;
    return true;
}
