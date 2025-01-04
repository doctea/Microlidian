#ifndef DEBUG__INCLUDED
#define DEBUG__INCLUDED

#include <Arduino.h>

void setup_debug_menu();

int freeRam();
void debug_free_ram();

void reset_rp2040();
void reset_upload_firmware();

#define Serial_println(X)   if(Serial)Serial.println(X)
#define Serial_printf(...)  if(Serial)Serial.printf(__VA_ARGS__)
#define Serial_print(X)     if(Serial)Serial.print(X)

#ifndef Serial_flush
    #ifdef SERIAL_FLUSH_REALLY
        #define Serial_flush() if(Serial)Serial.flush()
    #else
        #define Serial_flush() {}
    #endif
#endif
#ifndef Debug_printf
    #ifndef F
        #define F(x) x
    #endif
    #ifdef ENABLE_DEBUG_SERIAL
        #define Debug_println(X)    if(Serial)Serial.println(X)
        #define Debug_printf(...)   if(Serial)Serial.printf(__VA_ARGS__)
        #define Debug_print(X)      if(Serial)Serial.print(X)
    #else
        #define Debug_println(X)    {}
        #define Debug_printf(...)     {}
        #define Debug_print(X)      {}
    #endif
#endif

#include "menu_messages.h"

extern bool debug_flag;

volatile extern int menu_time;
volatile extern int tft_time;
volatile extern int missed_micros;
volatile extern int sequencer_time;

#endif
