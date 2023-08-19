#include "Config.h"
#ifdef ENABLE_SCREEN

#include "debug.h"

#include "menu.h"

#include "midi_usb/midi_usb_rp2040.h"

#ifdef ENABLE_CV_INPUT
    #include "cv_input.h"
    #ifdef ENABLE_CLOCK_INPUT_CV
        #include "cv_input_clock.h"
    #endif
#endif

#include "mymenu/screen.h"
#include "mymenu/menu_debug.h"

#include "core_safe.h"

#include <atomic>
extern std::atomic<bool> started;
//extern std::atomic<bool> menu_locked;
extern std::atomic<bool> ticked;
std::atomic<bool> frame_ready = false;

#include "menu.h"

//extern DisplayTranslator_Configured *tft;
//extern DisplayTranslator_Configured displaytranslator;

void setup_screen() {
    #ifdef ENABLE_SCREEN
        pinMode(ENCODER_KNOB_L, INPUT_PULLUP);
        pinMode(ENCODER_KNOB_R, INPUT_PULLUP);
        pinMode(PIN_BUTTON_A, INPUT_PULLDOWN);
        pinMode(PIN_BUTTON_B, INPUT_PULLDOWN);
    
        //tft = &displaytranslator; 
        //tft->init();

        tft_print((char*)"Ready!");
        tft_clear();

        setup_menu();

        Debug_println("About to init menu.."); Serial_flush();
        menu->start();
        Debug_printf("after menu->start(), free RAM is %u\n", freeRam());

        //setup_debug_menu();   // do this in setup() where we can control its position better
        //Serial.printf(F("after setup_debug_menu()\n")); //, free RAM is %u\n"), freeRam());

        menu->select_page(0);
        menu->auto_update = false;
    #endif
}


void push_display() {
    static unsigned long last_drawn;

    if (millis() - last_drawn < MENU_MS_BETWEEN_REDRAW) return;
    if (!menu->tft->ready()) return;
    if (!frame_ready) return;
    if (is_locked()) return;
    acquire_lock();
        
    //uint32_t interrupts = save_and_disable_interrupts();
    menu->updateDisplay();
    //restore_interrupts(interrupts);
    last_drawn = millis();

    frame_ready = false;

    release_lock();
}

void update_screen_dontcare() {
    update_screen();
}

void draw_screen() {
    //if (locked || menu==nullptr) 
    //    return;
    while (is_locked() || ticked || frame_ready) {
        delay(MENU_MS_BETWEEN_REDRAW/8);
    };
    //menu_locked = true;
    acquire_lock();
    //uint32_t interrupts = save_and_disable_interrupts();
    frame_ready = false;
    menu->display();
    frame_ready = true;
    //restore_interrupts(interrupts);
    release_lock();

    push_display();
}

void setup1() {
    while (!started) {
        delay(1);
    };
}

void loop1() {
    // doing this here instead of on first core means that two Microlidians powered up together won't clock drift badly
    #ifdef USE_TINYUSB
        USBMIDI.read();
    #endif
    
    static unsigned long last_pushed = 0;
    //if (last_pushed==0) delay(5000);
    while(is_locked()) {
        delay(MENU_MS_BETWEEN_REDRAW/8);
    }
    if (menu!=nullptr && millis() - last_pushed > MENU_MS_BETWEEN_REDRAW) {
        draw_screen();
        last_pushed = millis();
    }
    #ifdef ENABLE_CV_INPUT
        static unsigned long last_cv_update = 0;
        if (millis() - last_cv_update > time_between_cv_input_updates && !is_locked()) {
            acquire_lock();
            update_cv_input();
            release_lock();
            last_cv_update = millis();
        }
    #endif
}

#endif