// framework
#include <Arduino.h>

// needed for set_sys_clock_khz
#include "pico/stdlib.h"
// needed for irq_set_priority(), TIMER_IRQ_0..3, USBCTRL_IRQ
#include "hardware/irq.h"
//#include <intctl.h>
#include "hardware/timer.h"

// our app config
#include "Config.h"
#include "BootConfig.h"

#include "debug.h"

// midihelpers library clock handling
#include <clock.h>
#include <bpm.h>
#include <profiling.h>
//#include "midi_usb/midi_usb_rp2040.h"

#include "sequencer/sequencing.h"
#include "sequencer/Multi/MultiSequencer.h"
#include "sequencer/Euclidian/Sequencer.h"
#include "sequencer/Insects/AntTrailPattern.h"
#include "sequencer/TuringMachine/TuringMachinePattern.h"

#ifdef ENABLE_PROFILING
    static void setup_profiling();
#endif

#ifdef ENABLE_STORAGE
    #include "storage/storage.h"
    #include "saveload_test.h"
#endif

#ifdef ENABLE_SCREEN
    #include "mymenu/screen.h"
    #ifdef ENABLE_ACCENTS
        #include "mymenu/menuitems_accent.h"
    #endif
#endif

#ifdef ENABLE_CV_INPUT
    #include "cv_input.h"
    #ifdef ENABLE_CLOCK_INPUT_CV
        #include "cv_input_clock.h"
    #endif

    #include "outputs/output_voice.h"
#endif

#ifdef ENABLE_CV_OUTPUT
    #include "cv_output.h"
#endif

#include "outputs/output_processor.h"

#include "core_safe.h"

#include <atomic>
#include <SimplyAtomic.h>

#ifdef USE_UCLOCK_GENERIC
    void uClockCheckTime(uint32_t micros_time);
#endif

FLASHMEM void setup_parameter_outputs(IMIDICCTarget *);
FLASHMEM void create_midi_parameter_output_menu_items();

// todo: move this elsewhere?
RP2040DualMIDIOutputWrapper *output_wrapper;

std::atomic<bool> started = false;
std::atomic<bool> ticked = false;

void do_tick(uint32_t ticks);

// serial console to host, for debug etc
void setup_serial() {
    #ifdef WAIT_FOR_SERIAL
        Serial.begin(115200);
        Serial.setTimeout(0);

        while(!Serial) {};
    #endif
}

// call this when global clock should be reset
void global_on_restart() {
  set_restart_on_next_bar(false);

  //Serial.println(F("on_restart()==>"));
  clock_reset();
  last_processed_tick = -1;
  
  //send_midi_serial_stop_start();
  //behaviour_manager->on_restart();
  //Serial.println(F("<==on_restart()"));
}

void auto_handle_start();

void auto_handle_start_wrapper() {
    messages_log_add("auto_handle_start_wrapper()!");
    auto_handle_start();
}

#ifdef WAIT_FOR_SERIAL
    #define Debug_println(X)    if(Serial)Serial.println(X)
    #define Debug_printf(...)   if(Serial)Serial.printf(__VA_ARGS__)
    #define Debug_print(X)      if(Serial)Serial.print(X)
    #define F(X) X
#endif

// #ifdef ENABLE_PARAMETERS
//   repeating_timer_t parameter_timer;
//   bool parameter_repeating_callback(repeating_timer_t *rt) {
//     if (started) {
//       parameter_manager->throttled_update_cv_input__all(5, false, false);
//     }
//     return true;
//   }
// #endif

#ifdef USE_TINYUSB
  repeating_timer_t usb_timer;
  bool usb_repeating_callback(repeating_timer_t *rt) {
    while(USBMIDI.read()) {}
    return true;
  }
#endif

void setup() {
    Debug_println("setup() starting");
    Debug_printf("at start of setup(), free RAM is %u\n", freeRam());
    setup_profiling();

    // overclock the CPU so that we can afford all those CPU cycles drawing the UI!
    // 240mhz because, if we are to think about using the USB-Host-on-PIO thing, the system clock needs to be a multiple of 120mhz
    set_sys_clock_khz(240000, true);

    ticked = false;
    started = false;

    setup_serial();
    Debug_printf("after setup_serial(), free RAM is %u\n", freeRam());

    #ifdef USE_UCLOCK
        setup_uclock(do_tick, uClock.PPQN_24);

        // Claude told me that this would help reduce the jitter i was seeing due to 
        // " the FIFO being unable to drain while ATOMIC() holds interrupts disabled"
        // and maybe it even die work?
        // TODO: verify that this actually does something; move it to setup_uclock
        // Lower the priority of ALL hardware-timer alarm IRQs below the USB IRQ.
        // On RP2040/Cortex-M0+, USBCTRL_IRQ defaults to 0x80; timer alarms also default
        // to 0x80 (equal priority → no preemption).  By setting timer alarms to 0xC0
        // (lowest software level) the USB drain ISR can preempt the uClock ISR while it
        // is spin-waiting on a full TX FIFO, eliminating the multi-ms stalls in do_tick.
        irq_set_priority(TIMER1_IRQ_0, 0xC0);
        irq_set_priority(TIMER1_IRQ_1, 0xC0);
        irq_set_priority(TIMER1_IRQ_2, 0xC0);
        irq_set_priority(TIMER1_IRQ_3, 0xC0);
    #else
        setup_cheapclock();
    #endif
    set_global_restart_callback(global_on_restart);

    #if defined(ENABLE_CV_INPUT) && defined(ENABLE_CLOCK_INPUT_CV)
        set_check_cv_clock_ticked_callback(actual_check_cv_clock_ticked);
        set_clock_mode_changed_callback(clock_mode_changed);
    #endif

    setup_midi();
    Debug_printf("after setup_midi(), free RAM is %u\n", freeRam());
    #ifdef USE_TINYUSB
        setup_usb();
        Debug_printf("after setup_usb(), free RAM is %u\n", freeRam());
    #endif

    output_wrapper = new RP2040DualMIDIOutputWrapper();
    setup_output(output_wrapper, new ChosenDrumKitMIDIOutputProcessor(output_wrapper));
    Debug_printf("after setup_output(), free RAM is %u\n", freeRam());

    #ifdef ENABLE_PARAMETERS
        #ifdef ENABLE_CV_INPUT
            setup_cv_input();
            Debug_printf("after setup_cv_input(), free RAM is\t%u\n", freeRam());

            setup_parameter_inputs();
            Debug_printf("after setup_parameter_inputs(), free RAM is\t%u\n", freeRam());
        #endif
        #ifdef ENABLE_CV_INPUT  // these are midi outputs!
            setup_parameter_outputs(output_wrapper);
            Debug_printf("after setup_parameter_outputs(), free RAM is\t%u\n", freeRam());
        #endif
    #endif

    #ifdef ENABLE_SCREEN
        //delay(1000);    // see if giving 1 second to calm down will help reliability of screen initialisation... it does not. :(
        Debug_printf("before setup_screen(), free RAM is %u\n", freeRam());
        setup_screen(LOW);
        Debug_printf("after setup_screen(), free RAM is %u\n", freeRam());

        #ifdef ENABLE_STORAGE
            setup_storage_menu();
        #endif
    #endif

    // check if the two buttons are held down, if so, enter firmware reset mode as quickly as possible!
    #ifdef ENABLE_SCREEN
        pushButtonA.update(); 
        pushButtonB.update();
        if (pushButtonA.read() && pushButtonB.read()) {
            reset_upload_firmware();
        }

        pushButtonA.resetStateChange();
        pushButtonB.resetStateChange();
    #endif

    #ifdef ENABLE_EUCLIDIAN

        #ifdef ENABLE_ACCENTS
            global_accent_source = new StepAccentSource(TIME_SIG_MAX_STEPS_PER_BAR);
        #endif
        
        //Serial.println("setting up sequencer..");
        sequencer = new MultiSequencer();

        // set up Euclidian Sequencer and patterns, and add to MultiSequencer
        EuclidianSequencer *euclidian_sequencer = new EuclidianSequencer(output_processor->nodes);
        output_processor->configure_sequencer(euclidian_sequencer);
        euclidian_sequencer->initialise_patterns();
        euclidian_sequencer->reset_patterns();
        output_processor->setup_parameters();
        setup_output_processor_parameters();
        ((MultiSequencer*)sequencer)->addSequencer(euclidian_sequencer);

        // set up Insect Sequencer and patterns, and add to MultiSequencer
        SimpleSequencer *insect_sequencer = new SimpleSequencer(output_processor->nodes);
        // insect_sequencer->add_pattern(new AntTrailPattern(output_processor->nodes));
        // set up Turing Machine pattern
        // TODO: rename this, move it somewhere else, it's not really an "insect" pattern
        TuringMachinePattern *tm_pattern = new TuringMachinePattern(output_processor->nodes);
        tm_pattern->set_path_segment("pattern_0");
        tm_pattern->set_steps(16);
        tm_pattern->set_output(output_processor->get_output_for_label("Melody"));
        insect_sequencer->add_pattern(tm_pattern);
        ((MultiSequencer*)sequencer)->addSequencer(insect_sequencer);

        #if defined(ENABLE_PARAMETERS)
            parameter_manager->addInput(tm_pattern);

            //Serial.println("..calling sequencer.getParameters()..");
            LinkedList<FloatParameter*> *params = sequencer->getParameters();
            Debug_printf("after setting up sequencer parameters, free RAM is %u\n", freeRam());
        #endif

        Debug_printf("after setup_output_processor_parameters(), free RAM is\t%u\n", freeRam());
        #ifdef ENABLE_SCREEN
            #ifdef ENABLE_ACCENTS
                global_accent_source->make_menu_items();
            #endif

            sequencer->make_menu_items(menu, COMBINE_NONE);
            menu->select_page(0);   // todo: why do we do this?
            Debug_printf("after setting up sequencer and menus, free RAM is %u\n", freeRam());
        #endif
    #endif


    #if defined(ENABLE_SCREEN) && defined(ENABLE_PARAMETERS)
        //menu->add_page("Parameter Inputs");
        Debug_printf("before setup_parameter_menu(), free RAM is %u\n", freeRam());
        setup_parameter_menu();
        Debug_printf("after setup_parameter_menu(), free RAM is %u\n", freeRam());
    #endif

    #if defined(ENABLE_PARAMETERS) && defined(ENABLE_CV_INPUT)
        setup_cv_pitch_inputs();
        Debug_printf("after setup_cv_pitch_inputs(), free RAM is %u\n", freeRam());
    #endif

    #ifdef ENABLE_PARAMETERS
        parameter_manager->setDefaultParameterConnections();
    #endif

    #ifdef ENABLE_SCREEN
        #ifdef ENABLE_CV_INPUT
            Debug_printf("before setup_cv_pitch_inputs_menu(), free RAM is %u\n", freeRam());
            setup_cv_pitch_inputs_menu();
            Debug_printf("after setup_cv_pitch_inputs_menu(), free RAM is %u\n", freeRam()); Serial_flush();
        #endif
        #ifdef ENABLE_PARAMETERS
            Debug_printf("before create_midi_parameter_output_menu_items(), free RAM is %u\n", freeRam()); Serial_flush();
            //Serial.println("bouta create_midi_parameter_output_menu_items?"); Serial.flush();
            create_midi_parameter_output_menu_items();
            Debug_printf("after create_midi_parameter_output_menu_items(), free RAM is %u\n", freeRam()); Serial_flush();
        #endif
        //Serial.println("getting this far 2?"); Serial.flush();
        Debug_printf("before output_wrapper->create_menu_items(), free RAM is %u\n", freeRam()); Serial_flush();
        output_wrapper->create_menu_items();
        Debug_printf("before output_processor->create_menu_items(), free RAM is %u\n", freeRam()); Serial_flush();
        output_processor->create_menu_items();
        Debug_printf("after creating output wrapper and processor menuitems, free RAM is %u\n", freeRam()); Serial_flush();
        setup_debug_menu();
        Debug_printf("after setup_debug_menu(), free RAM is %u\n", freeRam());
        menu->setup_quickjump();

        menu->select_page(0);
    #endif

    #ifdef ENABLE_CV_OUTPUT
        setup_cv_output();
    #endif

    #ifdef LOAD_CALIBRATION_ON_BOOT
        parameter_manager->load_all_calibrations();
    #endif

    // setup the saveloadlib tree
    #ifdef ENABLE_STORAGE
        Serial.println("Setting up saveloadlib..."); Serial.flush();
        setup_saveloadlib();
        Serial.println("Finished setting up saveloadlib!"); Serial.flush();

        // load system settings from flash, if they exist
        Serial.println("Loading system settings..."); Serial.flush();
        load_system_settings();
        Serial.println("Finished loading system settings!"); Serial.flush();
        
        #ifdef ENABLE_TESTSAVELOAD
            test_object->create_menu_items();
        #endif
    #endif

    started = true;

    #ifdef DEBUG_ENVELOPES
        set_bpm(10);
    #endif

    #ifdef ENABLE_SCREEN
        menu->set_last_message((String("Started up, free RAM is ") + String(freeRam())).c_str());
    #endif

    Debug_printf("at end of setup(), free RAM is %u\n", freeRam());

    Debug_println("setup() finished!");

    #ifdef USE_UCLOCK
        //if (Serial) Serial.println("Starting uClock...");
        //Serial_flush();
        clock_reset();
        clock_start();
        //if (Serial) Serial.println("Started uClock!");
        //Serial_flush();
    #endif
    started = true;

    // set up repeating timers to process tasks
    // #ifdef ENABLE_PARAMETERS
    //     add_repeating_timer_ms(5, parameter_repeating_callback, nullptr, &parameter_timer);
    // #endif
    #ifdef USE_TINYUSB
        add_repeating_timer_us(250, usb_repeating_callback, nullptr, &usb_timer);
    #endif

}

//volatile bool ticked = false;

#define LOOP_AVG_COUNT 50
uint_fast32_t loop_averages[LOOP_AVG_COUNT];
uint_fast32_t loop_average = 0;
uint_fast8_t pos = 0;
void add_loop_length(int length) {
    loop_averages[pos] = length;
    pos++;
    if (pos>=LOOP_AVG_COUNT) {
        uint_fast32_t c = 0;
        for (uint_fast32_t i = 0 ; i < LOOP_AVG_COUNT ; i++) {
            c += loop_averages[i];
        }
        loop_average = c / LOOP_AVG_COUNT;
        pos = 0;
    }
}

void read_serial_buffer() {
    //uint32_t interrupts = save_and_disable_interrupts();
    if (Serial) {
        while (Serial.available()) {
            char c = Serial.read();
            if (c == 'd') {
                menu->knob_left();
                menu->send_frame = true;
            } else if (c == 'f') {
                menu->knob_right();
                menu->send_frame = true;
            } else if (c == 'a') {
                menu->button_back();
                menu->send_frame = true;
            } else if (c == 'b') {
                menu->button_select();
                menu->button_select_released();
                menu->send_frame = true;
            } else if (c == 'e') {
                menu->send_frame = true;
            } else if (c == 'L') {
                menu->send_frame = true;
                menu->send_frame_live = !menu->send_frame_live;
            }
            /*else {
                messages_log_add(String("Wanted a char like 'd' aka value for turning the knob left = ") + String((int)'d'));
                messages_log_add(String("got serial char: ") + String((int)c));
            }
            messages_log_add(String("got serial char: ") + c);*/
        }
        Serial.clearWriteError();
    }
    //restore_interrupts(interrupts);
}

bool menu_tick_pending = false;
//uint32_t menu_tick_pending_tick = -1;

// ── Profiling slots (declared at file scope so they are safe to use from the
//    uClock ISR, which calls do_tick — function-local statics have a guard
//    variable whose initialisation can race with the first ISR call.) ────────
PROFILE_SLOT_DECL(p_dotick,            "do_tick [total]");
PROFILE_SLOT_DECL(p_sequencer_ontick,  "do_tick seq::on_tick");
PROFILE_SLOT_DECL(p_outproc_process,   "do_tick outproc::process");
PROFILE_SLOT_DECL(p_cv_chord_process,  "do_tick cv_chord::process");
PROFILE_SLOT_DECL(p_menu_update_ticks, "loop menu::update_ticks");
PROFILE_SLOT_DECL(p_output_proc_loop,  "loop output_proc::loop");
PROFILE_SLOT_DECL(p_menu_update_inputs,"loop menu::update_inputs");

// Spike thresholds — set to ~2–3× observed average so only genuine outliers are
// captured.  Tune after a fresh profile_reset_all() + profile_print_all() run.
// Values are in microseconds.  Called from setup() so they're set before any tick fires.
FLASHMEM static void setup_profiling() {
    PROFILE_SET_SPIKE_THRESHOLD(p_dotick,             4000);  // avg ~1.7ms
    PROFILE_SET_SPIKE_THRESHOLD(p_sequencer_ontick,   3000);  // avg ~1.2ms
    PROFILE_SET_SPIKE_THRESHOLD(p_outproc_process,     400);  // avg ~140µs (fires every 16th)
    PROFILE_SET_SPIKE_THRESHOLD(p_cv_chord_process,    400);  // avg ~130µs
    PROFILE_SET_SPIKE_THRESHOLD(p_menu_update_ticks,  3500);  // avg ~1.8ms
    PROFILE_SET_SPIKE_THRESHOLD(p_output_proc_loop,    500);  // avg ~83µs  — 110× outlier needs investigation
    PROFILE_SET_SPIKE_THRESHOLD(p_menu_update_inputs,  200);  // avg ~14µs
    // Note: thresholds for the Core 1 rendering slots (draw_screen, cv_input_update)
    //       are set in screen.cpp since those slots are static to that translation unit.

    // Spike modulo — show tick%N alongside each spike so you can see which
    // phase of the clock cycle the outlier occurred on.  At PPQN=24:
    //   % 6  = position within a 16th note (0..5)
    //   % 24 = position within a beat      (0..23)
    //   % 96 = position within a bar       (0..95)
    PROFILE_SET_SPIKE_MODULO(p_dotick,             96);  // bar phase
    PROFILE_SET_SPIKE_MODULO(p_sequencer_ontick,   96);  // bar phase — key for finding pattern-rotation spikes
    PROFILE_SET_SPIKE_MODULO(p_outproc_process,     6);  // 16th phase — should always be 0
    PROFILE_SET_SPIKE_MODULO(p_cv_chord_process,    6);  // 16th phase — bimodal split investigation
    PROFILE_SET_SPIKE_MODULO(p_menu_update_ticks,  96);  // bar phase
    PROFILE_SET_SPIKE_MODULO(p_output_proc_loop,   96);  // last known tick at time of spike
}

void do_tick(uint32_t in_ticks) {
    PROFILE_SCOPE(p_dotick);
    PROFILE_SET_TICK(in_ticks);  // capture tick number for spike correlation
    #ifdef USE_UCLOCK
        ::ticks = in_ticks;
        // todo: hmm non-USE_UCLOCK mode doesn't actually use the in_ticks passed in here..?
    #endif
    //if (Serial) Serial.printf("ticked %u\n", ticks);
    if (is_restart_on_next_bar() && is_bpm_on_bar(ticks)) {
        //if (debug) Serial.println(F("do_tick(): about to global_on_restart"));
        global_on_restart();

        set_restart_on_next_bar(false);
    }

    output_wrapper->sendClock();

    #ifdef ENABLE_EUCLIDIAN
        if (sequencer->is_running()) {
            PROFILE_START(p_sequencer_ontick);
            sequencer->on_tick(ticks);
            PROFILE_STOP(p_sequencer_ontick);
        }
        if (is_bpm_on_sixteenth(ticks) && output_processor->is_enabled()) {
            PROFILE_START(p_outproc_process);
            output_processor->process();
            PROFILE_STOP(p_outproc_process);
        }
    #endif

    PROFILE_START(p_cv_chord_process);
    cv_chord_output_1->process();
    cv_chord_output_2->process();
    cv_chord_output_3->process();
    PROFILE_STOP(p_cv_chord_process);

    /*#ifdef ENABLE_CV_OUTPUT
        if (cv_output_enabled) {
            dac_output.write(0, parameter_manager->getInputForName("LFO sync")->get_normal_value_unipolar()*65535.0);
        }
    #endif*/

    //last_processed_tick = in_ticks;
}

void loop() {
    uint32_t mics_start = micros();
    //Serial.println("loop()"); Serial.flush();
    
    //tft_print("MAIN!");
    
    // doing this on the second core means that two Microlidians powered up at the same time won't drift too much
    // however, it causes crashes...
    // we do this with a repeating timer instead now
    // #ifndef PROCESS_USB_ON_SECOND_CORE
    //     #ifdef USE_TINYUSB
    //         ATOMIC()
    //         {
    //             USBMIDI.read();
    //         }
    //     #endif
    // #endif

    ATOMIC() 
    {
        ticked = update_clock_ticks();
    }

    #ifdef USE_UCLOCK
        // do_tick is called from interrupt via uClock, so we don't need to do it manually here
        #ifdef USE_UCLOCK_GENERIC
            uClockCheckTime(micros());
        #endif
    #else
        if (ticked) {
            ATOMIC() {
                do_tick(ticks);
            }
        }
    #endif

    #ifdef ENABLE_SCREEN
        static uint32_t last_tick = -1;
        static unsigned long last_drawn;
        /*if (ticked) {         // block updating menu if its currently being updated - works but very high BPMs (like 3000bpm!) have a lot of jitter
            while(locked) {}
            menu->update_ticks(ticks);
            last_tick = ticks;
        }*/
        ATOMIC() 
        {
            if ((ticked || menu_tick_pending) && !is_locked()) {     // don't block, assume that we can make up for the missed tick next loop; much less jitter when at very very high BPMs
                //acquire_lock();
                PROFILE_START(p_menu_update_ticks);
                menu->update_ticks(ticks);
                PROFILE_STOP(p_menu_update_ticks);
                //release_lock();
                last_tick = ticks;
                menu_tick_pending = false;
            } else if (ticked && is_locked()) {
                menu_tick_pending = true;
                //menu->update_ticks(ticks);
                //last_tick = ticks;
            }
        }
    #endif

    ATOMIC()
    {
        if (playing && output_processor->is_enabled()) {
            PROFILE_START(p_output_proc_loop);
            output_processor->loop();
            PROFILE_STOP(p_output_proc_loop);
        }
    }

    ATOMIC() 
    {
        if (playing && clock_mode==CLOCK_INTERNAL && last_ticked_at_micros>0 && micros() + loop_average >= last_ticked_at_micros + micros_per_tick) {
            // don't process anything else this loop, since we probably don't have time before the next tick arrives
            //Serial.printf("early return because %i + %i >= %i + %i\n", micros(), loop_average, last_ticked_at_micros, micros_per_tick);
            //Serial.flush();
        } else {
            read_serial_buffer();

            #ifdef ENABLE_SCREEN
            if (!is_locked()) {
                //acquire_lock();
                PROFILE_START(p_menu_update_inputs);
                menu->update_inputs();
                PROFILE_STOP(p_menu_update_inputs);
                //release_lock();
            }
            #endif

            add_loop_length(micros()-mics_start);
        }
    }

    ATOMIC() 
    {
        // if the back button is held down for 4 seconds, do a soft reboot
        #ifdef ENABLE_SCREEN
            if (!pushButtonA.isPressed() && pushButtonB.isPressed() && pushButtonB.currentDuration() >= 4000) {
                //#define AIRCR_Register (*((volatile uint32_t*)(PPB_BASE + 0x0ED0C)))
                //AIRCR_Register = 0x5FA0004;
                reset_rp2040();
            } else if (
                pushButtonA.isPressed() && pushButtonB.isPressed() && pushButtonA.currentDuration() >= 3000 && pushButtonA.currentDuration() >= 4000
            ) {
                reset_upload_firmware();
            }
        #endif
    }

    process_queued_file_output();

    //Serial.println("end of loop()"); Serial.flush();
}

