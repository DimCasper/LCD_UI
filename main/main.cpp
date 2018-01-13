#include <arduino.h>
#include <avr/pgmspace.h>
#include "keyDetect.h"
#include "mystack.h"
#include "LCDscreen.h"
#include "config.h"
#include "general.h"
#include "stringstack.h"

// #include <FreeStack.h>
//<Idle,MPos:0.000,0.000,0.000,WPos:0.000,0.000,0.000>

/**
 *  If you don't have this library, please download it at
 *  https://github.com/greiman/SdFat
 **/
#include <SdFat.h>

keyDetect key;

SdFat SD;
SdBaseFile runfile;
mystack<uint16_t> fileList;
stringstack path;

#define EOF -1

/** ===================================== **/

// act
void act();
void homing ();
void SDselect ();
void unlock ();

int getfilelist();
inline void path_popback();
// inline void path_back();

// main control
bool sendLine();
int fileCompare(const void* a,const void* b);

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
void statusPage();
void homing();
void SDselect();
void unlock();

void enterCallback_Status();

//目錄
// "顯示字串", 連結方法或子目錄, 是否含有子目錄, 子目錄長度
#define menu_0_length 4
PROGMEM const menuTable menu_0[] = {
{"Status page"          , (void*)statusPage, false,0},
{"Homing Setting"       , (void*)homing    , false,0},
{"Open SD card"         , (void*)SDselect  , false,0},
{"Unlock"               , (void*)unlock    , false,0}
};

void statusPage()
{
    // set callback function
    key.setCallback(KEY_DOWN, nullptr);
    key.setCallback(KEY_UP, nullptr);
    key.setCallback(KEY_ENTER, enterCallback_Status);

    flag_screen = STATUSPAGE;
    menuClear();
    menuInit(menu_0_length,menu_0);
    statusDisplay();
}

void menuPage()
{
    #ifdef DEBUG
    Serial.println(F("MenuPage"));
    #endif // DEBUG
    key.setCallback(KEY_UP,menuUp);
    key.setCallback(KEY_DOWN,menuDown);
    key.setCallback(KEY_ENTER,menuTableEnter);

    flag_screen = MENUPAGE;
    menuPrint();
    //menuClear();
    //menuInit(menu_0_length,menu_0);
}

void errorPage(String text)
{
    errorPage(text.c_str());
}

void errorPage(const char* text,bool datamem = NORMALDATA)
{
    key.setCallback(KEY_UP,nullptr);
    key.setCallback(KEY_DOWN,nullptr);
    key.setCallback(KEY_ENTER,menuPage);
    clear();
    if(datamem)
        printline_P(text);
    else
        printline(text);
}

bool readOK(String& t)
{
  return !strcmp_P(t.c_str(),PSTR("ok"));
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

void downCallback()
{

}

void upCallback()
{

}

void enterCallback_Status()
{
    #ifdef DEBUG
    Serial.println(F("Enter callback in status page."));
    #endif // DEBUG
    menuPage();
}

void enterCallback_File(int select)
{
    // enter selected directory or file.
    if(select==0)
    {
        menuInit();
        menuBack();
        if(path.size()==0)
        {
            menuPage();
        }
        else
        {
            path_popback();
            menuInit(getfilelist);
        }
    }
    else
    {
        SdBaseFile basefile;
        basefile.open(SD.vwd(),fileList[select-1],O_READ);
        if(basefile.isDir())
        {
            char temp[NAME_MAX_LENGTH];
            basefile.getName(temp,NAME_MAX_LENGTH-1);
            basefile.close();
            path.add(temp);
            SD.chdir(temp,true);
            menuInit(getfilelist);
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
            menuClear();

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
            statusPage();

            // initial timer
            run_start_time = millis();
        }
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

String getFileName(int index);

void SDselect ()
{
    flag_screen = FILEPAGE;
    if(!SD.begin())
    {
        #if defined(DEBUG)
        Serial.println(F("SD begin error."));
        #endif // defined
        errorPage(PSTR("No SD card."),PROGDATA);
        flag_screen = SDERRORPAGE;
        return ;
    }
    path.clear();
    menuInit(getfilelist(),getFileName,enterCallback_File,enterCallback_Status);
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

int getfilelist()
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
    return fileList.size();
    #if defined(DEBUG)

    #endif // defined
}

/** --- **/
//讀檔案(行讀取)讀一個位元組 EOF=end of file
inline void filereadline(FatFile& readfile,String& buffer)
{
    for(int temp=readfile.read() ; temp!=EOF && (temp!='\r' && temp!='\n') ; temp=readfile.read())
    {
        buffer += (char)temp;
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

// For LCDscreen menu control

String getFileName(int index)
{
    if(!index)
    {
        return PSTR("../");
    }
    index -= 1;
    #if defined(DEBUG)
    Serial.println(F("../"));
    #endif // defined
    char temp[NAME_MAX_LENGTH];
    SdBaseFile basefile;
    basefile.open(SD.vwd(),fileList[index],O_READ);
    basefile.getName(temp,NAME_MAX_LENGTH-2);
    if(basefile.isDir()) strcat_P(temp,PSTR("/"));
    basefile.close();
    #if defined(DEBUG)
    Serial.println(temp);
    #endif // DEBUG
    return String(temp);
}

/** ============ Arduino main ============ **/

void setup()
{
    // set grbl baud rate
    Serial.begin(BAUD_RATE);
    Serial.setTimeout(SERIAL_TIMEOUT);

    // print init to lcd
    LCDScreenInit();
    welcomeDisplay();

    // start
    delay(1000UL); // pause welcome scene at least 0.5s
    statusPage();
}

void loop ()
{
    act();

    key.detect();
}
