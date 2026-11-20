#pragma once

#include <Arduino.h>

void setup_debug_menu();

int freeRam();
void debug_free_ram();

void reset_rp2040();
void reset_upload_firmware();

#if defined(WAIT_FOR_SERIAL) && !defined(ENABLE_DEBUG_SERIAL)
    #define ENABLE_DEBUG_SERIAL
#endif

#include <shared_debug.h>

#include "menu_messages.h"

extern bool debug_flag;

volatile extern int menu_time;
volatile extern int tft_time;
volatile extern int missed_micros;
volatile extern int sequencer_time;

