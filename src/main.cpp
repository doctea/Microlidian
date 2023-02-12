#include <Arduino.h>

#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

#include "Encoder.h"
#include "Bounce2.h"

#include "Config.h"

#include "debug.h"

#include "bpm.h"

bool debug_flag = false;

/*#include <Adafruit_GFX_Buffer.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>*/

Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

#ifdef ENABLE_SCREEN
    #include "menu.h"
#endif

Encoder encoder(D2, D3);
Bounce pushButton = Bounce(D1, 10); // 10ms debounce

void setup() {
    Serial.begin(115200);
    //delay(1000);
    //while(!Serial) {}
    Serial.println("setup() starting");
    Serial.flush();

    pinMode(ENCODER_KNOB_L, INPUT_PULLUP);
    pinMode(ENCODER_KNOB_R, INPUT_PULLUP);
    pinMode(PIN_BUTTON_A, INPUT_PULLDOWN);

    #ifdef ENABLE_SCREEN
        //tft_print((char*)"Ready!"); 
        //tft_clear();

        setup_menu();

        Serial.println("About to init menu.."); Serial_flush();
        menu->start();
        Serial.printf("after menu->start(), free RAM is %u\n", freeRam());

        //setup_debug_menu();
        //Serial.printf(F("after setup_debug_menu()\n")); //, free RAM is %u\n"), freeRam());

        menu->select_page(0);
    #endif

    Serial.println("setup() finished!");
    
}


void loop() {
    if (millis() % (int)micros_per_tick == 0)
        ticks++;
    
    if (is_bpm_on_beat(ticks)) {
        Serial.println("beat!");
    }

    #ifdef ENABLE_SCREEN
        /*if (debug_flag) { Serial.println(F("about to do menu->update_ticks(ticks)")); Serial_flush(); }
        menu->update_ticks(ticks);
        if (debug_flag) { Serial.println(F("just did menu->update_ticks(ticks)")); Serial_flush(); }

        //tft_update(ticks);
        ///Serial.println("going into menu->display and then pausing 1000ms: "); Serial_flush();
        */
        static unsigned long last_drawn;
        bool screen_was_drawn = false;
        if (menu!=nullptr) {
            menu->update_inputs();
        } else {
            Serial.println("menu is nullptr!");
        }
        if (millis() - last_drawn > MENU_MS_BETWEEN_REDRAW) {
            //menu->debug = true;
            //Serial.println("gonna redraw..");
            //long before_display = millis();
            //if (debug_flag) { Serial.println("about to menu->display"); Serial_flush(); }
            if (debug_flag) menu->debug = true;
            menu->display(); //update(ticks);
            //if (debug_flag) { Serial.println("just did menu->display"); Serial_flush(); }
            //Serial.printf("display() took %ums..", millis()-before_display);
            last_drawn = millis();
            screen_was_drawn = true;
        }
      //delay(1000); Serial.println("exiting sleep after menu->display"); Serial_flush();
    #endif
    /*Serial.println("looped");
    Serial.flush();*/
    //delay(1000);
}