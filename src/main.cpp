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

#ifdef ENABLE_STORAGE
    #include "storage/storage.h"
#endif

#ifdef ENABLE_SCREEN
    #include "mymenu/screen.h"
#endif

#ifdef ENABLE_CV_INPUT
    #include "cv_input.h"
    #ifdef ENABLE_CLOCK_INPUT_CV
        #include "cv_input_clock.h"
    #endif
#endif

#include "outputs/output.h"
MIDIOutputProcessor output_processer = MIDIOutputProcessor(&USBMIDI);

// serial console to host, for debug etc
void setup_serial() {
    Serial.begin(115200);
    Serial.setTimeout(0);

    #ifdef WAIT_FOR_SERIAL
        while(!Serial) {};
    #endif
}

// call this when global clock should be reset
void global_on_restart() {
  set_restart_on_next_bar(false);

  //Serial.println(F("on_restart()==>"));
  ticks = 0;
  last_processed_tick = -1;
  
  //send_midi_serial_stop_start();
  //behaviour_manager->on_restart();
  //Serial.println(F("<==on_restart()"));
}

void setup() {
    // overclock the CPU so that we can afford all those CPU cycles drawing the UI!
    //set_sys_clock_khz(225000, true);
    //set_sys_clock_khz(230000, true);
    set_sys_clock_khz(200000, true);

    setup_serial();
    Debug_println("setup() starting");

    setup_cheapclock();
    set_global_restart_callback(global_on_restart);
    #if defined(ENABLE_CV_INPUT) && defined(ENABLE_CLOCK_INPUT_CV)
        set_check_cv_clock_ticked_callback(actual_check_cv_clock_ticked);
        set_clock_mode_changed_callback(clock_mode_changed);
    #endif

    setup_midi();
    setup_usb();

    #ifdef ENABLE_SCREEN
        setup_screen();
        Serial.printf("after setup_screen(), free RAM is %u\n", freeRam());
    #endif

    #ifdef ENABLE_STORAGE
        setup_storage();
        Serial.printf("after setup_storage(), free RAM is %u\n", freeRam());
    #endif    

    #ifdef ENABLE_CV_INPUT
        setup_cv_input();
        Serial.printf("after setup_cv_input(), free RAM is %u\n", freeRam());
        setup_parameters();
        Serial.printf("after setup_parameters(), free RAM is %u\n", freeRam());
    #endif

    #ifdef ENABLE_EUCLIDIAN
        //Serial.println("setting up sequencer..");
        setup_sequencer();
        output_processer.configure_sequencer(&sequencer);
        setup_sequencer_menu();
        Serial.printf("after setup_sequencer_menu, free RAM is %u\n", freeRam());
    #endif

    #if defined(ENABLE_CV_INPUT) && defined(ENABLE_EUCLIDIAN)
        //Serial.println("..calling getParameters()..");
        LinkedList<FloatParameter*> *params = sequencer.getParameters();
        parameter_manager->addParameters(params);
        params->clear();
        delete params;
        Serial.printf("after setting up sequencer parameters, free RAM is %u\n", freeRam());
    #endif

    #ifdef ENABLE_CV_INPUT
        parameter_manager->setDefaultParameterConnections();
    #endif

    #if defined(ENABLE_SCREEN) && defined(ENABLE_CV_INPUT)
        menu->add_page("Parameter Inputs");
        Serial.printf("before setup_parameter_menu(), free RAM is %u\n", freeRam());
        setup_parameter_menu();
        Serial.printf("after setup_parameter_menu(), free RAM is %u\n", freeRam());
    #endif

    setup_debug_menu();
    debug_free_ram();

    menu->select_page(0);

    started = true;

    Serial.printf("at end of setup(), free RAM is %u\n", freeRam());

    Debug_println("setup() finished!");
}

//volatile bool ticked = false;

#define LOOP_AVG_COUNT 50
int loop_averages[LOOP_AVG_COUNT];
int loop_average = 0;
int pos = 0;
void add_loop_length(int length) {
    loop_averages[pos] = length;
    pos++;
    if (pos>=LOOP_AVG_COUNT) {
        int c = 0;
        for (int i = 0 ; i < LOOP_AVG_COUNT ; i++) {
            c += loop_averages[i];
        }
        loop_average = c / LOOP_AVG_COUNT;
        pos = 0;
    }
}

void read_serial_buffer() {
    //uint32_t interrupts = save_and_disable_interrupts();
    if (Serial) {
        Serial.read();
        Serial.clearWriteError();
    }
    //restore_interrupts(interrupts);
}

bool menu_tick_pending = false;
//uint32_t menu_tick_pending_tick = -1;

void loop() {
    uint32_t mics_start = micros();
    //Serial.println("loop()");
    
    USBMIDI.read();

    ticked = update_clock_ticks();

    if (ticked) {
        if (is_restart_on_next_bar() && is_bpm_on_bar(ticks)) {
            //if (debug) Serial.println(F("do_tick(): about to global_on_restart"));
            global_on_restart();

            set_restart_on_next_bar(false);
        }

        USBMIDI.sendClock();
        DINMIDI.sendClock();

        #ifdef ENABLE_EUCLIDIAN
            sequencer.on_tick(ticks);
            if (is_bpm_on_sixteenth(ticks)) {
                output_processer.process();
            }
        #endif
    }

    #ifdef ENABLE_SCREEN
        static uint32_t last_tick = -1;
        static unsigned long last_drawn;
        /*if (ticked) {         // block updating menu if its currently being updated - works but very high BPMs (like 3000bpm!) have a lot of jitter
            while(locked) {}
            menu->update_ticks(ticks);
            last_tick = ticks;
        }*/
        if ((ticked || menu_tick_pending) && !locked) {     // don't block, assume that we can make up for the missed tick next loop; much less jitter when at very very high BPMs
            menu->update_ticks(ticks);
            last_tick = ticks;
            menu_tick_pending = false;
        } else if (ticked && locked) {
            menu_tick_pending = true;
            //menu->update_ticks(ticks);
            //last_tick = ticks;
        }
    #endif

    if (clock_mode==CLOCK_INTERNAL && last_ticked_at_micros>0 && micros() + loop_average >= last_ticked_at_micros + micros_per_tick) {
        // don't process anything else this loop, since we probably don't have time before the next tick arrives
        //Serial.printf("early return because %i + %i >= %i + %i\n", micros(), loop_average, last_ticked_at_micros, micros_per_tick);
        //Serial.flush();
    } else {
        //read_serial_buffer();

        if (!locked)
            menu->update_inputs();

        add_loop_length(micros()-mics_start);
    }

}

