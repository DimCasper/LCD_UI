#ifndef CONFIG_H
#define CONFIG_H

 #define DEBUG

#define BAUD_RATE 115200
#define SERIAL_TIMEOUT 100
// #defint SERIAL_CONECT_TO_COMPUTER

/** LCD **/

// #define LCD_IIC
#define LCD_PARALLEL_4
// #define LCD_PARALLEL_8

#if defined(LCD_PARALLEL_4) || defined(LCD_PARALLEL_8)
    #define LCD_RS 19 //
    #define LCD_EN 18 //
    #define LCD_D4 17 //
    #define LCD_D5 16 //
    #define LCD_D6 15 //
    #define LCD_D7 14 //
#if defined(LCD_PARALLEL_8)
    #define LCD_D0
    #define LCD_D1
    #define LCD_D2
    #define LCD_D3
#endif // LCD 8 line
#endif // 4 line and 8 line common

//#define LCD_2004
 #define LCD_12864

/** Key detect **/

#define COUNTDOWN_BY_KEYDETECT
// #define USE_ENCODER

/** Define the cycle length(ms) of anything. **/

#define KEY_DETECT_CYCLE 10
#define STATUS_SCREEN_CYCLE 1000

/** SD card and File **/

#define NAME_MAX_LENGTH 64

#endif // CONFIG_H
