#include <Arduino.h>

#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

#include "Encoder.h"
#include "Bounce2.h"

#include "Config.h"

#include "BootConfig.h"

#include "debug.h"

#include <clock.h>
#include <bpm.h>

#include "sequencer/sequencing.h"

bool update_screen();
void update_screen_dontcare();

bool debug_flag = false;

Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

#ifdef ENABLE_SCREEN
    #include "menu.h"
#endif

Encoder encoder(D2, D3);
Bounce pushButton = Bounce(D1, 10); // 10ms debounce

#include "outputs/output.h"
MIDIOutputProcessor output_processer = MIDIOutputProcessor(&MIDI);

struct perf_t {
    unsigned long average_loop_length = 0;
};
perf_t perf_record;

void calc_loop_length(unsigned long start, unsigned long finish) {
    perf_record.average_loop_length += finish-start;
    perf_record.average_loop_length /= 2;
}

void setup_serial() {
    Serial.begin(115200);
    Serial.setTimeout(0);

    #ifdef WAIT_FOR_SERIAL
        while(!Serial) {};
    #endif
}

void setup_usb() {
    #if defined(ARDUINO_ARCH_MBED) && defined(ARDUINO_ARCH_RP2040)
        // Manual begin() is required on core without built-in support for TinyUSB such as mbed rp2040
        TinyUSB_Device_Init(0);
    #endif

    while( !TinyUSBDevice.mounted() ) delay(1);
}

void setup_midi() {
    MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.turnThruOff();

    MIDI.setHandleClock(pc_usb_midi_handle_clock);
    MIDI.setHandleStart(pc_usb_midi_handle_start);
    MIDI.setHandleStop(pc_usb_midi_handle_stop);
    MIDI.setHandleContinue(pc_usb_midi_handle_continue);
}

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

        //setup_debug_menu();
        //Serial.printf(F("after setup_debug_menu()\n")); //, free RAM is %u\n"), freeRam());

        menu->select_page(0);
    #endif
}

void setup() {
    setup_serial();
    Debug_println("setup() starting");

    setup_midi();

    setup_usb();

    setup_screen();

    Debug_println("setting up sequencer..");
    setup_sequencer();
    output_processer.configure_sequencer(&sequencer);

    Debug_println("setup() finished!");
}

volatile bool ticked = false;

void loop() {
    //Serial.println("loop()");
    uint32_t interrupts = save_and_disable_interrupts();
    MIDI.read();
    restore_interrupts(interrupts);

    //bool
    interrupts = save_and_disable_interrupts();
    ticked = update_clock_ticks();
    restore_interrupts(interrupts);
    //queue_try_add()

    if (ticked) {
        sequencer.on_tick(ticks);
        uint32_t interrupts = save_and_disable_interrupts();
        if (is_bpm_on_sixteenth(ticks)) {
            output_processer.process();
        }
        restore_interrupts(interrupts);

        //menu->update_ticks(ticks);
    }

    interrupts = save_and_disable_interrupts();
    if (Serial) {
        Serial.read();
        Serial.clearWriteError();
    }
    restore_interrupts(interrupts);

/*#ifdef DUALCORE
}

void loop1() {
#endif*/

    static uint32_t last_tick = -1;
    #ifdef ENABLE_SCREEN
        if (ticked) {    
            uint32_t interrupts = save_and_disable_interrupts();
            menu->update_ticks(ticks);
            last_tick = ticks;
            restore_interrupts(interrupts);
        }
        //multicore_launch_core1(update_screen_dontcare);
        update_screen();
        menu->tft->setCursor(0,0);
        menu->tft->printf("loop: %i", perf_record.average_loop_length);
    #endif
}


void update_display() {
    menu->updateDisplay();
}

void update_screen_dontcare() {
    update_screen();
}

bool update_screen() {
    bool screen_was_drawn = false;
    #ifdef ENABLE_SCREEN
        /*if (debug_flag) { Serial.println(F("about to do menu->update_ticks(ticks)")); Serial_flush(); }
        menu->update_ticks(ticks);
        if (debug_flag) { Serial.println(F("just did menu->update_ticks(ticks)")); Serial_flush(); }

        //tft_update(ticks);
        ///Serial.println("going into menu->display and then pausing 1000ms: "); Serial_flush();
        */
        static unsigned long last_drawn;
        if (menu!=nullptr) {
            uint32_t interrupts = save_and_disable_interrupts();
            menu->update_inputs();
            restore_interrupts(interrupts);
        } else {
            Debug_println("menu is nullptr!");
        }
        if (millis() - last_drawn > MENU_MS_BETWEEN_REDRAW) {
            //menu->debug = true;
            //Serial.println("gonna redraw..");
            //long before_display = millis();
            //if (debug_flag) { Serial.println("about to menu->display"); Serial_flush(); }
            if (!menu->tft->ready())
                return false;
            if (debug_flag) menu->debug = true;
            uint32_t interrupts = save_and_disable_interrupts();
            menu->auto_update = false;
            menu->display();
            restore_interrupts(interrupts);
            //multicore_launch_core1(update_display);
            update_display();
            //if (debug_flag) { Serial.println("just did menu->display"); Serial_flush(); }
            //Serial.printf("display() took %ums..", millis()-before_display);
            last_drawn = millis();
            screen_was_drawn = true;
        } else {
            screen_was_drawn = false;
        }
      //delay(1000); Serial.println("exiting sleep after menu->display"); Serial_flush();
    #endif

    return screen_was_drawn;
}
