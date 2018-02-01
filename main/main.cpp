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

char serial_buffer[SERIAL_BUFFER_LENGTH+1];
size_t serial_buffer_size = 0;
void (*serialCallback)();

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
void block();
void reset();

void enterCallback_Status();

//目錄
// "顯示字串", 連結方法或子目錄, 是否含有子目錄, 子目錄長度
#define menu_0_length 6
const menuTable menu_0[] PROGMEM = {
{"Status page"          , (void*)statusPage, false,0},
{"Homing Setting"       , (void*)homing    , false,0},
{"Open SD card"         , (void*)SDselect  , false,0},
{"Unlock"               , (void*)unlock    , false,0},
{"Stop"                 , (void*)block     , false,0},
{"Reset"                , (void*)reset     , false,0}
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
    menuPrint(menuTableGetLine);
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

int index_of(char* str,char cha,int index = 0)
{
    if(index<0) return;
    for(;*str;index++)
    {
        if(*(str+index)==cha)
        {
            return index;
        }
    }
    return -1;
}

int intParse(char* str)
{
    int i=0;
    for(;(*str)>='0'&&(*str)<='9';str++)
        i=i*10+((*str)-'0');
    return i;
}

bool statParse(char* str,status* obj)
{
    int index=0;
    if(~(index = index_of(str,'<'))) return false;
    index++;
    for(int i=0;i<STAT_LENGTH+1&&(str[index]!='|'||str[index]!=',');i++,index++)
    {
        obj->stat[i] = str[index];
    }
    if(~(index = index_of(str,'M',index))) return false;
    if(~(index = index_of(str,':',index))) return false;
    index++;
    obj->x = intParse(str+index);
    if(~(index = index_of(str,',',index))) return false;
    index++;
    obj->y = intParse(str+index);
    if(~(index = index_of(str,',',index))) return false;
    index++;
    obj->z = intParse(str+index);
    return true;
}

bool readOK(char* t)
{
  return !strcmp_P(t,PSTR("ok"));
}

void serialBufferClear()
{
    serial_buffer_size = 0;
}

void serialEvent()
{
    #ifdef DEBUG
    Serial.println(F("SerialEvent"));
    #endif // DEBUG
    if(Serial.peek()=='\r'||Serial.peek()=='\n'||serial_buffer_size==SERIAL_BUFFER_LENGTH)
    {
        Serial.read();
        #ifdef DEBUG
        Serial.println(F("\\t or \\n"));
        #endif // DEBUG
        if(!serial_buffer_size) return;
        serial_buffer[serial_buffer_size] = '\0';
        #ifdef DEBUG
        Serial.println(serial_buffer);
        #endif // DEBUG
        if(serialCallback)
        {
            serialCallback();
        }
        serialBufferClear();
        return ;
    }
    //讀取資料
    if(serial_buffer_size<SERIAL_BUFFER_LENGTH)
    {
        serial_buffer[serial_buffer_size] = Serial.read();
        serial_buffer_size++;
    }
}

void readError()
{
    statusPage();
    printline(serial_buffer,4);
    runfile.close();
    #if defined(DEBUG)
    Serial.println(F("ERROR."));//讀ERROR(需更改
    Serial.println(serial_buffer);
    #endif // define
    bitSet(flag_controlState,STANDBY);
    bitClear(flag_controlState,RUNNING);
    bitClear(flag_controlState,WAITING_RESPONSE);

    serialCallback = nullptr;
    delay(10);
}

/** ======= Callback function ======= **/

void readOkCallback()
{
    #ifdef DEBUG
    Serial.println(F("readOkCallback"));
    #endif // DEBUG
    //讀旗標
    if(!bitRead(flag_controlState,WAITING_RESPONSE))
    {
        //serialBufferClear();
        return ;
    }

    if(readOK(serial_buffer))//讀OK
    {
        bitClear(flag_controlState,WAITING_RESPONSE);
        serialCallback = nullptr;
        delay(10);
    }
    else
    {
        readError();
    }
}

void readStatusCallback()
{
    status temp;
    if(statParse(serial_buffer,&temp))
    {
        ;
    }
}

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
    #ifdef DEBUG
    Serial.println(F("enterCallback_File"));
    #endif // DEBUG
    // enter selected directory or file.
    if(select==0)
    {
        #ifdef DEBUG
        Serial.println(F("Select 0"));
        #endif // DEBUG
        if(path.size()==0)
        {
            #ifdef DEBUG
            Serial.println(F("No more path"));
            #endif // DEBUG
            menuPage();
        }
        else
        {
            #ifdef DEBUG
            Serial.println(F("Back to"));
            #endif // DEBUG
            path_popback();
        }
        menuBack();
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
            if(!SD.chdir(temp,true))
            {
                flash();
                return ;
            }
            path.add(temp);
            menuInit(getfilelist());
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
            serialCallback = readOkCallback;

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
            //statusSet(STATUS_RUN);
            customtime(millis()-run_start_time);
            progressBar(runfile.curPosition()*100/runfile.fileSize());
        }
        else
        {
            customtime(0);
            //statusSet(STATUS_IDLE);
        }
        if(!bitRead(flag_controlState,WAITING_RESPONSE))
        {
            //getStat();
        }
    }
}

void homing ()
{
    if(bitRead(flag_controlState,WAITING_RESPONSE)||bitRead(flag_controlState,RUNNING)) return;
    Serial.println(F("$H"));
    bitSet(flag_controlState, WAITING_RESPONSE);
    serialCallback = readOkCallback;
}

void unlock ()
{
    if(bitRead(flag_controlState,WAITING_RESPONSE)||bitRead(flag_controlState,RUNNING)) return;
    Serial.println(F("$X"));
    bitSet(flag_controlState, WAITING_RESPONSE);
    serialCallback = readOkCallback;
}

void block()
{
    flag_controlState = 0;
}

void reset ()
{
    Serial.write(24); // ctrl+x grbl soft reset
    flag_controlState = 0;
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
    menuInit(getfilelist()+1,getFileName,enterCallback_File,enterCallback_Status);
    key.setCallback(KEY_ENTER,menuEnter);
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
    getfilelist();
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
    serialCallback = readOkCallback;
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
        char temp[5];
        strcpy_P(temp,PSTR("../"));
        #if defined(DEBUG)
        Serial.println(F("../"));
        #endif // defined
        return String(temp);
    }
    index -= 1;
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
    serialCallback = serialBufferClear;

    // print init to lcd
    LCDScreenInit();
    welcomeDisplay();

    /*Serial.print((int)menu_0);
    Serial.print('\t');
    Serial.println((int)((void*)menu_0+6*sizeof(menuTable)));
    menuTable tt;
    memcpy_P(&tt,(void*)menu_0+5*sizeof(menuTable),sizeof(menuTable));
    for(int i=0;i<sizeof(menuTable);i++)
    {
        Serial.print(reinterpret_cast<const __FlashStringHelper *>(tt.title));
        Serial.print('\t');
        if(!((i+1)%sizeof(menuTable))) Serial.println();
    }
    for(int i=0;i<menu_0_length*sizeof(menuTable);i++)
    {
        Serial.print(int(pgm_read_byte((void*)menu_0+i)));
        Serial.print('\t');
        if(!((i+1)%sizeof(menuTable))) Serial.println();
    }
    while(1) ;*/

    // start
    delay(1000UL); // pause welcome scene at least 0.5s
    statusPage();
}

void loop ()
{
    act();

    key.detect();
}
