#include "Config.h"
#ifdef ENABLE_SCREEN

#include "debug.h"

#include "menu.h"

#include "mymenu/screen.h"
#include "mymenu/menu_debug.h"

Encoder encoder(D2, D3);
Bounce pushButton = Bounce(D1, 10); // 10ms debounce

std::atomic<bool> ticked = false;

void setup_screen() {
    #ifdef ENABLE_SCREEN
        pinMode(ENCODER_KNOB_L, INPUT_PULLUP);
        pinMode(ENCODER_KNOB_R, INPUT_PULLUP);
        pinMode(PIN_BUTTON_A, INPUT_PULLDOWN);
        pinMode(PIN_BUTTON_B, INPUT_PULLDOWN);

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

#include <atomic>
std::atomic<bool> locked;
std::atomic<bool> frame_ready;

void push_display() {
    static unsigned long last_drawn;

    if (millis() - last_drawn < MENU_MS_BETWEEN_REDRAW) return;
    if (!menu->tft->ready()) return;
    if (!frame_ready) return;
    if (locked) return;
    locked = true;
        
    uint32_t interrupts = save_and_disable_interrupts();
    menu->updateDisplay();
    restore_interrupts(interrupts);
    last_drawn = millis();

    frame_ready = false;
    locked = false;
}

void update_screen_dontcare() {
    update_screen();
}

void draw_screen() {
    //if (locked || menu==nullptr) 
    //    return;
    while (locked || ticked || frame_ready) {
        delay(MENU_MS_BETWEEN_REDRAW/2);
    };
    locked = true;
    uint32_t interrupts = save_and_disable_interrupts();
    frame_ready = false;
    menu->display();
    frame_ready = true;
    restore_interrupts(interrupts);
    locked = false;    
}

void loop1() {
    static unsigned long last_pushed = 0;
    //if (last_pushed==0) delay(5000);
    while(locked) {
        delay(MENU_MS_BETWEEN_REDRAW/2);
    }
    if (menu!=nullptr && millis() - last_pushed > MENU_MS_BETWEEN_REDRAW) {
        draw_screen();
        last_pushed = millis();
    }
}

bool update_screen() {
    return false;
    bool screen_was_drawn = false;
    #ifdef ENABLE_SCREEN
        /*if (debug_flag) { Serial.println(F("about to do menu->update_ticks(ticks)")); Serial_flush(); }
        menu->update_ticks(ticks);
        if (debug_flag) { Serial.println(F("just did menu->update_ticks(ticks)")); Serial_flush(); }

        //tft_update(ticks);
        ///Serial.println("going into menu->display and then pausing 1000ms: "); Serial_flush();
        */       
        if (locked) 
            return false;
        locked = true;

        static unsigned long last_drawn;
        /*if (menu!=nullptr) {
            uint32_t interrupts = save_and_disable_interrupts();
            menu->update_inputs();
            restore_interrupts(interrupts);
        } else {
            Debug_println("menu is nullptr!");
        }*/       
        if (!locked && millis() - last_drawn > MENU_MS_BETWEEN_REDRAW) {
            //menu->debug = true;
            //Serial.println("gonna redraw..");
            //long before_display = millis();
            //if (debug_flag) { Serial.println("about to menu->display"); Serial_flush(); }
            if (!menu->tft->ready())
                return false;
            if (debug_flag) menu->debug = true;
            uint32_t interrupts = save_and_disable_interrupts();
            menu->auto_update = false;
            int t = millis();
            //menu->display();
            //draw_screen();
            menu_time = (menu_time + (millis()-t))/2;
            restore_interrupts(interrupts);
            //multicore_launch_core1(push_display);
            t = millis();
            push_display();
            tft_time = (tft_time + (millis()-t))/2;
            //if (debug_flag) { Serial.println("just did menu->display"); Serial_flush(); }
            //Serial.printf("display() took %ums..", millis()-before_display);
            last_drawn = millis();
            screen_was_drawn = true;
        } else {
            screen_was_drawn = false;
        }
        locked = false;
      //delay(1000); Serial.println("exiting sleep after menu->display"); Serial_flush();
    #endif

    return screen_was_drawn;
}


#endif