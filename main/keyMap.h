#ifndef KEYMAP_H_INCLUDED
#define KEYMAP_H_INCLUDED

#include "config.h"

/***********************************************************
 ** You can define your pin of key here.
 ** Make sure the number of pin grater than 0
 ** If you did not use the key, just leave it or commit it
 ***********************************************************/

#if defined( USE_ENCODER )
    #define ENCODER_A_PIN 2
    #define ENCODER_B_PIN 3
#endif // USE_ENCODER

#define KEY_UP_PIN 2
#define KEY_DOWN_PIN 3
#define KEY_LEFT_PIN
#define KEY_RIGHT_PIN

#define KEY_ENTER_PIN 4
#define KEY_CANCEL_PIN

#define KEY_CUSTOM1_PIN
#define KEY_CUSTOM2_PIN

// #define NUMBER_GRATER_THAN_8
// #define NUMBER_GRATER_THAN_16

enum keyMap {
KEY_NONE=-1,

/** Arrow Key **/
#if KEY_UP_PIN-1 > -1 || defined(USE_ENCODER)
KEY_UP,
#endif // KEY_UP_PIN
#if KEY_DOWN_PIN-1 > -1 || defined(USE_ENCODER)
KEY_DOWN,
#endif // KEY_DOWN_PIN
#if KEY_LEFT_PIN-1 > -1
KEY_LEFT,
#endif // KEY_LEFT_PIN
#if KEY_RIGHT_PIN-1 > -1
KEY_RIGHT,
#endif // KEY_RIGHT_PIN

/** Control Key **/
#if KEY_ENTER_PIN-1 > -1
KEY_ENTER,
#endif // KEY_ENTER_PIN
#if KEY_CANCEL_PIN-1 > -1
KEY_CANCEL,
#endif // KEY_CANCEL_PIN

/** Custom Key **/
#if KEY_CUSTOM1_PIN-1 > -1
KEY_CUSTOM1,
#endif // KEY_CUSTOM1_PIN
#if KEY_CUSTOM2_PIN-1 > -1
KEY_CUSTOM2,
#endif // KEY_CUSTOM2_PIN

NUMBER_OF_KEY
};

#endif // KEYMAP_H_INCLUDED
