#include "keyDetect.h"
#include "keyMap.h"
#include "config.h"
#include <limits.h>
#include <arduino.h>

// #define DEBUG

keyDetect::keyDetect()
{
    for(uint8_t i=0;i<NUMBER_OF_KEY;i++)
    {
        callbackFunction[i] = NULL;
    }
    init();
}

keyDetect::keyDetect(void (*callbackfun[])())
{
    for(uint8_t i=0;i<NUMBER_OF_KEY && i<sizeof(callbackfun)/sizeof(callbackfun[0]);i++)
    {
        callbackFunction[i] = callbackfun[i];
    }
    init();
}

void keyDetect::init()
{
    #if defined(USE_ENCODER)
    pinMode(ENCODER_A_PIN, INPUT_PULLUP);
    pinMode(ENCODER_B_PIN, INPUT_PULLUP);
    encoder_motion = 0;
    #endif // defined

    flag = 0;

    /** Arrow Key **/
    #if KEY_UP_PIN-1 > -1
    pinMode(KEY_UP_PIN, INPUT_PULLUP);
    #endif // KEY_UP_PIN
    #if KEY_DOWN_PIN-1 > -1
    pinMode(KEY_DOWN_PIN, INPUT_PULLUP);
    #endif // KEY_DOWN_PIN
    #if KEY_LEFT_PIN-1 > -1
    pinMode(KEY_LEFT_PIN, INPUT_PULLUP);
    #endif // KEY_LEFT_PIN
    #if KEY_RIGHT_PIN-1 > -1
    pinMode(KEY_RIGHT_PIN, INPUT_PULLUP);
    #endif // KEY_RIGHT_PIN

    /** Control Key **/
    #if KEY_ENTER_PIN-1 > -1
    pinMode(KEY_ENTER_PIN, INPUT_PULLUP);
    #endif // KEY_ENTER_PIN
    #if KEY_CANCEL_PIN-1 > -1
    pinMode(KEY_CANCEL_PIN, INPUT_PULLUP);
    #endif // KEY_CANCEL_PIN

    /** Custom Key **/
    #if KEY_CUSTOM1_PIN-1 > -1
    KEY_CUSTOM1_PIN, INPUT_PULLUP);
    #endif // KEY_CUSTOM1_PIN
    #if KEY_CUSTOM2_PIN-1 > -1
    KEY_CUSTOM2_PIN, INPUT_PULLUP);
    #endif // KEY_CUSTOM2_PIN


    #ifdef NUMBER_GRATER_THAN_16
    flag = ULONG_MAX;
    #elif defined NUMBER_GRATER_THAN_8
    flag = UINT_MAX;
    #else
    flag = UCHAR_MAX;
    #endif // NUMBER_GRATER_THAN_16
    #ifdef COUNTDOWN_BY_KEYDETECT
    timer = millis();
    #endif // COUNTDOWN_BY_KEYDETECT
}

void keyDetect::setCallback(keyMap key,void (*callbackfun)())
{
    callbackFunction[key] = callbackfun;
}

#define CHECK(A) \
        if(bitRead(flag,A) && !digitalRead(A ## _PIN)) \
        { \
            if(callbackFunction[A] != NULL) callbackFunction[A](); \
            bitClear(flag,A); \
        } \
        if(digitalRead(A ## _PIN)) \
        { \
            bitSet(flag,A); \
        }

void keyDetect::detect()
{
    #ifdef COUNTDOWN_BY_KEYDETECT
    if(millis()-timer<KEY_DETECT_CYCLE) return ;
    timer = millis();
    #endif // COUNTDOWN_BY_KEYDETECT

    #if defined(USE_ENCODER)
    checkEncoder();
    #endif // defined
    #if defined(KEY_UP_PIN) && (KEY_UP_PIN-1)>-1
    CHECK(KEY_UP);
    #endif // USE KEY_UP
    #if defined(KEY_DOWN_PIN) && (KEY_DOWN_PIN-1)>-1
    CHECK(KEY_DOWN);
    #endif // USE KEY_DOWN
    #if defined(KEY_LEFT_PIN) && (KEY_LEFT_PIN-1)>-1
    CHECK(KEY_LEFT);
    #endif // USE KEY_LEFT
    #if defined(KEY_RIGHT_PIN) && (KEY_RIGHT_PIN-1)>-1
    CHECK(KEY_RIGHT);
    #endif // USE KEY_RIGHT

    #if defined(KEY_ENTER_PIN) && (KEY_ENTER_PIN-1)>-1
    CHECK(KEY_ENTER);
    #endif // USE KEY_ENTER
    #if defined(KEY_CANCEL_PIN) && (KEY_CANCEL_PIN-1)>-1
    CHECK(KEY_CANCEL);
    #endif // USE KEY_CANCEL

    #if defined(KEY_CUSTOM1_PIN) && (KEY_CUSTOM1_PIN-1)>-1
    CHECK(KEY_CUSTOM1);
    #endif // USE KEY_CUSTOM1
    #if defined(KEY_CUSTOM2_PIN) && (KEY_CUSTOM2_PIN-1)>-1
    CHECK(KEY_CUSTOM2);
    #endif // USE KEY_CUSTOM2
}

#undef CHECK(A)

#if defined(USE_ENCODER)
    #define ENCODER_A KEY_UP
    #define ENCODER_B KEY_DOWN
#endif // Use encoder

#if defined(USE_ENCODER)
#define COMBINE(a,b) (a##b)
#define flagRead(FLAG,POS) bitRead(FLAG,COMBINE(ENCODER_,POS))
inline void keyDetect::checkEncoder ()
{
    uint8_t readFlag = (digitalRead(ENCODER_A_PIN)<<ENCODER_A) | (digitalRead(ENCODER_B_PIN)<<ENCODER_B);
    if(flagRead(flag,A)!=flagRead(readFlag,A) || flagRead(flag,B)!=flagRead(readFlag,B))
    {
        if((flagRead(flag,B)&&flagRead(readFlag,A)) || (!flagRead(flag,B)&&!flagRead(readFlag,A)))
        {
          encoder_motion++;
        }
        else
        {
          encoder_motion--;
        }
        bitWrite(flag,ENCODER_A,flagRead(readFlag,A));
        bitWrite(flag,ENCODER_B,flagRead(readFlag,B));
        if(flagRead(flag,A) && flagRead(flag,B))
        {
            //Serial.println(encoder_motion>0?"CCW":"CW");
            if(encoder_motion>0)
            {
                if(callbackFunction[KEY_UP]) callbackFunction[KEY_UP]();
            }
            else
            {
                if(callbackFunction[KEY_DOWN]) callbackFunction[KEY_DOWN]();
            }
            encoder_motion = 0;
        }
    }
}

#undef COMBINE
#undef flagRead
#endif
