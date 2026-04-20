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
#include <profiling.h>
extern std::atomic<bool> started;
//extern std::atomic<bool> menu_locked;
extern std::atomic<bool> ticked;
std::atomic<bool> frame_ready = false;

// ── Profiling slots for Core 1 hot paths ───────────────────────────────────────────────
// NOTE: These slots are written exclusively from Core 1 (loop1/draw_screen),
// while profile_print_all() is triggered from Core 0.  Results are approximate
// (no atomics), but good enough for profiling purposes.
PROFILE_SLOT_DECL(p_draw_screen,       "draw_screen [total]");
PROFILE_SLOT_DECL(p_cv_input_update,   "loop1 cv_input_update");
// Spike thresholds for Core 1 slots (set at file-scope startup time)
static bool _prof_screen_thresholds_init = []() {
    PROFILE_SET_SPIKE_THRESHOLD(p_draw_screen,     19000);  // avg ~16.7ms
    PROFILE_SET_SPIKE_THRESHOLD(p_cv_input_update, 13000);  // avg ~8.9ms
    // No clock modulo for these — they're free-running on Core 1.
    // spike_modulo = 96 gives the last tick Core 0 was on when this fired
    // (approximate — cross-core, no lock), useful for rough correlation.
    PROFILE_SET_SPIKE_MODULO(p_draw_screen,     96);
    PROFILE_SET_SPIKE_MODULO(p_cv_input_update, 96);
    return true;
}();

#include "menu.h"

void setup_menu(bool pressed_state = HIGH);

void setup_screen(bool pressed_state) {
    #ifdef ENABLE_SCREEN
        pinMode(ENCODER_KNOB_L, INPUT_PULLUP);
        pinMode(ENCODER_KNOB_R, INPUT_PULLUP);
        pinMode(PIN_BUTTON_A, INPUT);
        pinMode(PIN_BUTTON_B, INPUT);

        tft_print((char*)"Ready!");
        tft_clear();

        Debug_println("About to setup_menu.."); 
        setup_menu(pressed_state);

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

    // disabling locking here makes no apparent difference to clock drift, but seems risky since if the screen update takes a long time then the next tick might come in while we're still updating the screen, which could cause all sorts of weirdness; so maybe best to leave locking enabled here just in case.
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
    PROFILE_SCOPE(p_draw_screen);
    //if (locked || menu==nullptr) 
    //    return;
    //while (is_locked() || ticked || frame_ready) {
        /*#if defined(PROCESS_USB_ON_SECOND_CORE) && defined(USE_TINYUSB)
            // doing this here instead of on first core means that two Microlidians powered up together won't clock drift badly
            //ATOMIC() {
                USBMIDI.read();
            //}
        #endif*/
        //delay(MENU_MS_BETWEEN_REDRAW/8);
    //};
    //menu_locked = true;
    acquire_lock();
    frame_ready = false;
    // EXPERIMENTAL (2026-04-19): removed ATOMIC() wrapper around menu->display() so that
    // USB serial TX ISR can service serial frame streaming for remote viewer.
    // Previously this was: ATOMIC() { menu->display(); frame_ready = true; }
    // REVERT THIS if crashes occur with serial connected and the queued serial dispatch
    // approach in main.cpp does not fix them.
    ATOMIC() {
        menu->display();
        frame_ready = true;
    }
    release_lock();

    push_display();
}

void setup1() {
    while (!started) {
        delay(1);
    };
    //while (true) {}   // with display enabled, and do_tick set to early return, clock loses around 40ms every second
}

#include "cv_output.h"

void loop1() {  

    #ifdef PROCESS_USB_ON_SECOND_CORE
        // doing this here instead of on first core means that two Microlidians powered up together won't clock drift badly
        #ifdef USE_TINYUSB
            USBMIDI.read();
        #endif
    #endif

    // with draw_screen locking disabled, and with this block disabled, clock loses about 15ms every second.
    #ifdef ENABLE_CV_INPUT
        static unsigned long last_cv_update = 0;
        if (cv_input_enabled) {
            if (parameter_manager->ready_for_next_update() && !is_locked()) {
                parameter_manager->process_calibration();

                acquire_lock();
                //ATOMIC() 
                {
                PROFILE_START(p_cv_input_update);
                parameter_manager->throttled_update_cv_input__all(time_between_cv_input_updates, false, false);
                PROFILE_STOP(p_cv_input_update);
                }
                release_lock();
                last_cv_update = millis();
            }
        }
    #endif

    /*
    #ifdef ENABLE_CV_OUTPUT
        if (!calibrating && cv_output_enabled && !is_locked()) {
            acquire_lock();
            static int8_t current_pitch = 0;
            static uint32_t last_ticked = 0;
            //if (ticks < last_ticked) 
            //    current_pitch = 0;
            //if (ticks - last_ticked >= (PPQN)) {
            if (ticks != last_ticked) {
                last_ticked = ticks;

                //current_pitch = (ticks / PPQN) % MIDI_MAX_NOTE;
                current_pitch = ((BPM_CURRENT_BAR % 10) * 12);
                
                float calculated_voltage = get_voltage_for_pitch(current_pitch);
                //float calculated_voltage = parameter_manager->getInputForName("LFO sync")->get_normal_value_unipolar() * 10.0;//*65535.0;
                //Serial.printf("Calculated voltage %3.3f => ", calculated_voltage);
                calculated_voltage = get_calibrated_voltage(calculated_voltage);
                //Serial.printf("%3.3f\n", calculated_voltage);

                calculated_voltage /= 10.0;
                
                //acquire_lock();
                dac_output.write(
                    0,                
                    65535 - (calculated_voltage * 65535.0)
                );
                dac_output.write(
                    1,                
                    65535 - (calculated_voltage * 65535.0)
                );
                //release_lock();
            }
            release_lock();
        } else if (calibrating) {
            Serial.println("Starting calibration..."); Serial.flush();
            acquire_lock();

            calibrate_unipolar_minimum(0, "A");
            calibrate_unipolar_maximum(0, "A");

            calibrating = false;
            Serial.println("Finished calibration!"); Serial.flush();
            release_lock();
        }
    #endif
    */

    static unsigned long last_pushed = 0;
    //if (last_pushed==0) delay(5000);
    //while(is_locked()) {
    //    delay(MENU_MS_BETWEEN_REDRAW/16);

        /*#ifdef PROCESS_USB_ON_SECOND_CORE
            // doing this here instead of on first core means that two Microlidians powered up together won't clock drift badly
            // however, think it causes a deadlock if we don't process MIDI while waiting for a lock..?
            #ifdef USE_TINYUSB
                USBMIDI.read();
            #endif
        #endif*/
    //}
    ATOMIC() {
        if (!is_locked() && menu!=nullptr && millis() - last_pushed > MENU_MS_BETWEEN_REDRAW) {
            draw_screen();
            last_pushed = millis();

            /*if (ticks % 12 == 0 && Serial) {
                // attempt to push the framebuffer out over serial
                //Serial.println("pushing framebuffer.."); Serial.flush();
                //menu->tft->push_framebuffer_serial();
            }*/
        }
    }

}

#endif