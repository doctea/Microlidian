#include "Config.h"
#ifdef ENABLE_SCREEN

#include "debug.h"

#include "menu.h"

#ifdef ENABLE_CV_INPUT
    #include "cv_input.h"
    #ifdef ENABLE_CLOCK_INPUT_CV
        #include "cv_input_clock.h"
    #endif
#endif

#include "mymenu/screen.h"
#include "mymenu/menu_debug.h"

Encoder encoder(D2, D3);
Bounce pushButton = Bounce(D1, 10); // 10ms debounce


#include <atomic>
extern std::atomic<bool> started;
extern std::atomic<bool> menu_locked;
extern std::atomic<bool> ticked;
std::atomic<bool> frame_ready = false;

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


void push_display() {
    static unsigned long last_drawn;

    if (millis() - last_drawn < MENU_MS_BETWEEN_REDRAW) return;
    if (!menu->tft->ready()) return;
    if (!frame_ready) return;
    if (menu_locked) return;
    menu_locked = true;
        
    //uint32_t interrupts = save_and_disable_interrupts();
    menu->updateDisplay();
    //restore_interrupts(interrupts);
    last_drawn = millis();

    frame_ready = false;
    menu_locked = false;
}

void update_screen_dontcare() {
    update_screen();
}

void draw_screen() {
    //if (locked || menu==nullptr) 
    //    return;
    while (menu_locked || ticked || frame_ready) {
        delay(MENU_MS_BETWEEN_REDRAW/8);
    };
    menu_locked = true;
    //uint32_t interrupts = save_and_disable_interrupts();
    frame_ready = false;
    menu->display();
    frame_ready = true;
    //restore_interrupts(interrupts);
    menu_locked = false;    

    push_display();
}

void setup1() {
    while (!started) {
        delay(1);
    };
}

void loop1() {
    static unsigned long last_pushed = 0;
    //if (last_pushed==0) delay(5000);
    while(menu_locked) {
        delay(MENU_MS_BETWEEN_REDRAW/8);
    }
    if (menu!=nullptr && millis() - last_pushed > MENU_MS_BETWEEN_REDRAW) {
        draw_screen();
        last_pushed = millis();
    }
    #ifdef ENABLE_CV_INPUT
        static unsigned long last_cv_update = 0;
        if (!menu_locked && millis() - last_cv_update > time_between_cv_input_updates) {
            update_cv_input();
            last_cv_update = millis();
        }
    #endif
}

#endif