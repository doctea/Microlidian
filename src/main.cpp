#include <Arduino.h>

#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

#include "Encoder.h"
#include "Bounce2.h"

#include "Config.h"

#include <Adafruit_GFX_Buffer.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

#ifdef ENABLE_SCREEN
    #include "menu.h"
#endif

Encoder encoder(D2, D3);
Bounce pushButton = Bounce(D1, 10); // 10ms debounce

void setup() {
    Serial.begin(115200);
    Serial.println("setup() starting");

    #ifdef ENABLE_SCREEN
        tft_print((char*)"Ready!"); 
        tft_clear();

        Serial.println(F("About to init menu..")); Serial_flush();
        menu->start();
        //Serial.printf(F("after menu->start(), free RAM is %u\n"), freeRam());
        //tft_start();

        //setup_debug_menu();
        //Serial.printf(F("after setup_debug_menu()\n")); //, free RAM is %u\n"), freeRam());

        menu->select_page(0);
    #endif

    Serial.println("setup() finished!");
}


void loop() {
    Serial.println("looped");
    delay(1000);
}