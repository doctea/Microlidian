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

#ifdef ENABLE_CV_INPUT
    #include "cv_input.h"
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

    #ifdef ENABLE_CV_INPUT
        setup_cv_input();
        Serial.printf("after setup_cv_input(), free RAM is %u\n", freeRam());
        setup_parameters();
        Serial.printf("after setup_parameters(), free RAM is %u\n", freeRam());
    #endif

    Serial.println("setting up sequencer..");
    setup_sequencer();
    output_processer.configure_sequencer(&sequencer);

    #ifdef ENABLE_CV_INPUT
        Serial.println("..calling getParameters()..");
        LinkedList<FloatParameter*> *params = sequencer.getParameters();
        parameter_manager->addParameters(params);
        parameter_manager->setDefaultParameterConnections();
        params->clear();
        delete params;
    #endif

    #if defined(ENABLE_SCREEN) && defined(ENABLE_CV_INPUT)
        menu->add_page("Parameter Inputs");
        setup_parameter_menu();
        Serial.printf("after setup_parameter_menu(), free RAM is %u\n", freeRam());

        setup_debug_menu();

        menu->select_page(0);
    #endif

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
        bool screen_was_drawn = update_screen();
        //menu->tft->setCursor(0,0);
        //menu->tft->printf("loop: %i", perf_record.average_loop_length);
    #endif

    #ifdef ENABLE_CV_INPUT
      static unsigned long time_of_last_param_update = 0;
      if (!screen_was_drawn && millis() - time_of_last_param_update > TIME_BETWEEN_CV_INPUT_UPDATES) {
        if(debug_flag) parameter_manager->debug = true;
        if(debug_flag) Serial.println(F("about to do parameter_manager->update_voltage_sources()..")); Serial_flush();
        parameter_manager->update_voltage_sources();
        //if(debug) Serial.println("just did parameter_manager->update_voltage_sources().."); Serial_flush();
        //if(debug) Serial.println("about to do parameter_manager->update_inputs().."); Serial_flush();
        parameter_manager->update_inputs();
        //if(debug) Serial.println("about to do parameter_manager->update_mixers().."); Serial_flush();
        parameter_manager->update_mixers();
        if(debug_flag) Serial.println(F("just did parameter_manager->update_inputs()..")); Serial_flush();
        time_of_last_param_update = millis();
      }
    #endif
}

