// framework
#include <Arduino.h>

// needed for set_sys_clock_khz
#include "pico/stdlib.h"

// our app config
#include "Config.h"
#include "BootConfig.h"

#include "debug.h"

// midihelpers library clock handling
#include <clock.h>
#include <bpm.h>

#include "midi/midi_usb.h"

#include "sequencer/sequencing.h"

#ifdef ENABLE_SCREEN
    #include "mymenu/screen.h"
#endif

#include "outputs/output.h"
MIDIOutputProcessor output_processer = MIDIOutputProcessor(&MIDI);

// serial console to host, for debug etc
void setup_serial() {
    Serial.begin(115200);
    Serial.setTimeout(0);

    #ifdef WAIT_FOR_SERIAL
        while(!Serial) {};
    #endif
}


void setup() {
    // overclock the CPU so that we can afford all those CPU cycles drawing the UI!
    set_sys_clock_khz(225000, true);

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

    interrupts = save_and_disable_interrupts();
    ticked = update_clock_ticks();
    restore_interrupts(interrupts);

    if (ticked) {
        sequencer.on_tick(ticks);
        uint32_t interrupts = save_and_disable_interrupts();
        if (is_bpm_on_sixteenth(ticks)) {
            output_processer.process();
        }
        restore_interrupts(interrupts);
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
        update_screen();
        //menu->tft->setCursor(0,0);
        //menu->tft->printf("loop: %i", perf_record.average_loop_length);
    #endif
}

