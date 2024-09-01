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

#include "SimplyAtomic.h"
#include <atomic>
extern std::atomic<bool> started;
//extern std::atomic<bool> menu_locked;
extern std::atomic<bool> ticked;
std::atomic<bool> frame_ready = false;

#include "menu.h"

void setup_menu(bool pressed_state = HIGH);

void setup_screen() {
    #ifdef ENABLE_SCREEN
        pinMode(ENCODER_KNOB_L, INPUT_PULLUP);
        pinMode(ENCODER_KNOB_R, INPUT_PULLUP);
        pinMode(PIN_BUTTON_A, INPUT_PULLDOWN);
        pinMode(PIN_BUTTON_B, INPUT_PULLDOWN);

        tft_print((char*)"Ready!");
        tft_clear();

        Debug_println("About to setup_menu.."); 
        setup_menu(HIGH);

        tft_print("HUP!");      // <3 roo
        Debug_println("About to menu->updateDisplay() for the first time..");
        menu->updateDisplay();

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
    menu->updateDisplay();
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
        /*#if defined(PROCESS_USB_ON_SECOND_CORE) && defined(USE_TINYUSB)
            // doing this here instead of on first core means that two Microlidians powered up together won't clock drift badly
            //ATOMIC() {
                USBMIDI.read();
            //}
        #endif*/
        //delay(MENU_MS_BETWEEN_REDRAW/8);
    };
    //menu_locked = true;
    acquire_lock();
    //uint32_t interrupts = save_and_disable_interrupts();
    frame_ready = false;
    ATOMIC() {
        menu->display();
        frame_ready = true;
    }
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

    #ifdef PROCESS_USB_ON_SECOND_CORE
        // doing this here instead of on first core means that two Microlidians powered up together won't clock drift badly
        #ifdef USE_TINYUSB
            USBMIDI.read();
        #endif
    #endif

    static unsigned long last_pushed = 0;
    //if (last_pushed==0) delay(5000);
    while(is_locked()) {
        delay(MENU_MS_BETWEEN_REDRAW/8);

        /*#ifdef PROCESS_USB_ON_SECOND_CORE
            // doing this here instead of on first core means that two Microlidians powered up together won't clock drift badly
            // however, think it causes a deadlock if we don't process MIDI while waiting for a lock..?
            #ifdef USE_TINYUSB
                USBMIDI.read();
            #endif
        #endif*/
    }
    ATOMIC() {
        if (menu!=nullptr && millis() - last_pushed > MENU_MS_BETWEEN_REDRAW) {
            draw_screen();
            last_pushed = millis();
        }
    }
    #ifdef ENABLE_CV_INPUT
        static unsigned long last_cv_update = 0;
        if (cv_input_enabled) {
            if (parameter_manager->ready_for_next_update() && !is_locked()) {
                acquire_lock();
                //ATOMIC() 
                {
                    parameter_manager->throttled_update_cv_input__all(time_between_cv_input_updates, false, false);
                }
                release_lock();
                last_cv_update = millis();
            }
        }
    #endif
}

#endif