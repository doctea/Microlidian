#include "Config.h"
//#include "storage.h"

#include "debug.h"

#include <Arduino.h>

#include "sequencer/sequencing.h"
#include "sequencer/Euclidian.h"

#ifdef ENABLE_SCREEN

/*#include <Adafruit_GFX_Buffer.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>*/

#include "mymenu.h"

#include "submenuitem_bar.h"

/*#include "mymenu/menu_looper.h"
#include "mymenu/menu_sequencer.h"*/
#include "mymenu/menu_bpm.h"
#include "mymenu/menu_clock_source.h"

#include "mymenu/menuitems_scale.h"
/*#include "mymenu/menu_midi_matrix.h"

#include "menuitems_object_multitoggle.h"

#include "mymenu/menu_usb.h"
#include "mymenu/menu_behaviours.h"

#include "behaviours/behaviour_beatstep.h"
#include "behaviours/behaviour_keystep.h"
#include "behaviours/behaviour_mpk49.h"
#include "behaviours/behaviour_subclocker.h"
#include "behaviours/behaviour_craftsynth.h"
#include "behaviours/behaviour_neutron.h"

#include "midi/midi_out_wrapper.h"
#include "midi/midi_outs.h"
#include "midi/midi_pc_usb.h"

#include "behaviours/behaviour_subclocker.h"

#include "clock.h"

#include "profiler.h"*/

//DisplayTranslator *tft;
DisplayTranslator_Configured *tft = nullptr;
Menu *menu = nullptr; // = Menu();

#ifdef ENCODER_KNOB_L
    Encoder knob(ENCODER_KNOB_L, ENCODER_KNOB_R);
    //extern Encoder knob;
#endif
#ifdef PIN_BUTTON_A
    Bounce pushButtonA = Bounce(PIN_BUTTON_A, 10); // 10ms debounce
    //extern Bounce pushButtonA;
#endif
#ifdef PIN_BUTTON_B
    Bounce pushButtonB = Bounce(PIN_BUTTON_B, 10); // 10ms debounce
    //extern Bounce pushButtonB; 
#endif
#ifdef PIN_BUTTON_C
    Bounce pushButtonC = Bounce(PIN_BUTTON_C, 10); // 10ms debounce
    //extern Bounce pushButtonC;
#endif

LoopMarkerPanel top_loop_marker_panel = LoopMarkerPanel(LOOP_LENGTH_TICKS, PPQN, BEATS_PER_BAR, BARS_PER_PHRASE);

BPMPositionIndicator posbar = BPMPositionIndicator();
ClockSourceSelectorControl clock_source_selector = ClockSourceSelectorControl("Clock source", clock_mode);

/*#include "menuitems_fileviewer.h"
extern FileViewerMenuItem *sequence_fileviewer;
extern FileViewerMenuItem *project_fileviewer;*/

//MenuItem test_item_1 = MenuItem("test 1");

//DisplayTranslator_STeensy steensy = DisplayTranslator_STeensy();
//DisplayTranslator_STeensy_Big steensy = DisplayTranslator_STeensy_Big();
DisplayTranslator_Configured displaytranslator = DisplayTranslator_Configured();

/*#ifndef GDB_DEBUG
FLASHMEM 
#endif*/

float bpm_selector_values[] = { 60, 90, 120, 150, 180, 500, 1000, 2000, 3000 };
uint32_t external_cv_ticks_per_pulse_values[] = { 1, 2, 3, 4, 6, 8, 12, 16, 24 };
void set_external_cv_ticks_per_pulse_values(uint32_t new_value) {
    external_cv_ticks_per_pulse = new_value;
    //reset_clock();
    ticks = 0;
}
uint32_t get_external_cv_ticks_per_pulse_values() {
    return external_cv_ticks_per_pulse;
}

void setup_menu() {
    Debug_println(F("Starting setup_menu()..")); Serial_flush();

    tft = &displaytranslator; 
    tft->init();

    for (int x = 0 ; x < tft->width() ; x+=3) {
        for (int y = 0 ; y < tft->height() ; y+=3) {
            tft->drawLine(0,0,x,y,random(65535));
        }        
    }

    //delay(50);
    //Serial.println(F("Finished  constructor"));
    Serial_flush();
    Debug_println(F("Creating Menu object..")); Serial_flush();
    menu = new Menu(tft);
    Debug_println(F("Created Menu object")); Serial_flush();

    menu->set_screen_height_cutoff(0.5f);

    menu->set_messages_log(messages_log);

    menu->add_pinned(&top_loop_marker_panel);  // pinned position indicator
    menu->add(&posbar);                        // bpm and position indicator
    menu->add(&clock_source_selector);         // midi clock source (internal or from PC USB)

    // add start/stop/continue bar
    SubMenuItemBar *project_startstop = new SubMenuItemBar("Transport");
    project_startstop->add(new ActionItem("Start",    clock_start));
    project_startstop->add(new ActionItem("Stop",     clock_stop));
    project_startstop->add(new ActionItem("Continue", clock_continue));
    project_startstop->add(new ActionFeedbackItem("Restart", (ActionFeedbackItem::setter_def_2)set_restart_on_next_bar_on, is_restart_on_next_bar, "Restarting..", "Restart"));
    menu->add(project_startstop);

    #ifdef ENABLE_CLOCK_INPUT_CV
        SelectorControl<uint32_t> *external_cv_ticks_per_pulse_selector = new SelectorControl<uint32_t>("External CV clock: Pulses per tick");
        external_cv_ticks_per_pulse_selector->available_values = external_cv_ticks_per_pulse_values;
        external_cv_ticks_per_pulse_selector->num_values = sizeof(external_cv_ticks_per_pulse_values)/sizeof(uint32_t);
        external_cv_ticks_per_pulse_selector->f_setter = set_external_cv_ticks_per_pulse_values;
        external_cv_ticks_per_pulse_selector->f_getter = get_external_cv_ticks_per_pulse_values;
        menu->add(external_cv_ticks_per_pulse_selector);
    #endif

    // debug bpm selector
    SelectorControl<float> *bpm_selector = new SelectorControl<float>("BPM");
    bpm_selector->available_values = bpm_selector_values;
    bpm_selector->num_values = sizeof(bpm_selector_values)/sizeof(float);
    bpm_selector->f_setter = set_bpm;
    bpm_selector->f_getter = get_bpm;
    menu->add(bpm_selector);

    menu->add_page("Scales");
    menu->add(new ScaleMenuItem("Scales"));

    //menu->add(&test_item_1);

    #ifdef ENABLE_EUCLIDIAN
        menu->add_page("Mutation");
        menu->add(new ObjectNumberControl<EuclidianSequencer,float>("Density", &sequencer, &EuclidianSequencer::set_density,        &EuclidianSequencer::get_density, nullptr, MINIMUM_DENSITY, MAXIMUM_DENSITY));
        menu->add(new ObjectToggleControl<EuclidianSequencer>("Mutate", &sequencer, &EuclidianSequencer::set_mutated_enabled,       &EuclidianSequencer::is_mutate_enabled));
        menu->add(new ObjectToggleControl<EuclidianSequencer>("Reset", &sequencer,  &EuclidianSequencer::set_reset_before_mutate,   &EuclidianSequencer::should_reset_before_mutate));
        menu->add(new ObjectToggleControl<EuclidianSequencer>("Add phrase", &sequencer, &EuclidianSequencer::set_add_phrase_enabled,&EuclidianSequencer::is_add_phrase_enabled));
        menu->add(new ObjectToggleControl<EuclidianSequencer>("Fills", &sequencer,  &EuclidianSequencer::set_fills_enabled,         &EuclidianSequencer::is_fills_enabled));
        menu->add(new ObjectNumberControl<EuclidianSequencer,int>("Seed", &sequencer, &EuclidianSequencer::set_euclidian_seed,      &EuclidianSequencer::get_euclidian_seed));
    #endif
    
    #ifdef DISABLED_STUFF
    menu->add(new SeparatorMenuItem("Project"));

    ActionConfirmItem *project_save = new ActionConfirmItem("Save settings", &save_project_settings);
    ObjectNumberControl<Project,int> *project_selector = new ObjectNumberControl<Project,int>(
        "Project number", 
        project, 
        &Project::setProjectNumber, 
        &Project::getProjectNumber, 
        nullptr, 
        0, 
        100
    );

    menu->add(project_save);       // save project settings button
    menu->add(project_selector);   // save project selector button

    // project loading options (whether to load or hold matrix settings, clock, sequence, behaviour options)
    project_multi_recall_options = new ObjectMultiToggleControl("Recall options", true);
    MultiToggleItemClass<Project> *load_matrix = new MultiToggleItemClass<Project> (
        "MIDI Mappings",
        project,
        &Project::setLoadMatrixMappings,
        &Project::isLoadMatrixMappings
    );
    #ifdef ENABLE_CLOCKS
        MultiToggleItemClass<Project> *load_clock = new MultiToggleItemClass<Project> (
            "Clock Settings",
            project,
            &Project::setLoadClockSettings,
            &Project::isLoadClockSettings    
        );
    #endif
    #ifdef ENABLE_SEQUENCER
        MultiToggleItemClass<Project> *load_sequence = new MultiToggleItemClass<Project> (
            "Sequence Settings",
            project,
            &Project::setLoadSequencerSettings,
            &Project::isLoadSequencerSettings    
        );
    #endif
    MultiToggleItemClass<Project> *load_behaviour_settings = new MultiToggleItemClass<Project> {
        "Behaviour Options",
        project,
        &Project::setLoadBehaviourOptions,
        &Project::isLoadBehaviourOptions
    };
    project_multi_recall_options->addItem(load_matrix);
    #ifdef ENABLE_CLOCKS
        project_multi_recall_options->addItem(load_clock);
    #endif
    #ifdef ENABLE_SEQUENCER
        project_multi_recall_options->addItem(load_sequence);
    #endif
    project_multi_recall_options->addItem(load_behaviour_settings);
    //menu->add(&project_load_matrix_mappings);
    menu->add(project_multi_recall_options);

    // options for whether to auto-advance looper/sequencer/beatstep
    project_multi_autoadvance = new ObjectMultiToggleControl("Auto-advance", true);
    #ifdef ENABLE_SEQUENCER
        MultiToggleItemClass<Project> *auto_advance_sequencer = new MultiToggleItemClass<Project> (
            "Sequence",
            project,
            &Project::set_auto_advance_sequencer,
            &Project::is_auto_advance_sequencer
        );
        project_multi_autoadvance->addItem(auto_advance_sequencer);
    #endif
    #ifdef ENABLE_LOOPER
        MultiToggleItemClass<Project> *auto_advance_looper = new MultiToggleItemClass<Project> (
            "Looper",
            project,
            &Project::set_auto_advance_looper,
            &Project::is_auto_advance_looper
        );
        project_multi_autoadvance->addItem(auto_advance_looper);
    #endif
    #if defined(ENABLE_BEATSTEP) && defined(ENABLE_BEATSTEP_SYSEX)
        project_multi_autoadvance->addItem(new MultiToggleItemClass<DeviceBehaviour_Beatstep> (
            "Beatstep",
            behaviour_beatstep,
            &DeviceBehaviour_Beatstep::set_auto_advance_pattern,
            &DeviceBehaviour_Beatstep::is_auto_advance_pattern
        ));
    #endif
    menu->add(project_multi_autoadvance);

    project_fileviewer = new FileViewerMenuItem("Project");
    menu->add(project_fileviewer);

    menu->add_page("MIDI");
    menu->add(new SeparatorMenuItem("MIDI"));
    menu->add(new ObjectActionItem<MIDIMatrixManager>("{PANIC}", midi_matrix_manager, &MIDIMatrixManager::stop_all_notes));
    menu->add(&midi_matrix_selector);
    
    /*Serial.println(F("...starting behaviour_manager#make_menu_items..."));
    behaviour_manager->create_all_behaviour_menu_items(menu);
    Serial.println(F("...finished behaviour_manager#make_menu_items..."));*/

    // sequencer
    #ifdef ENABLE_SEQUENCER
        menu->add_page("Sequencer");
        //menu->add(&project_auto_advance_sequencer);
        menu->add(new SeparatorMenuItem("Sequencer"));
        menu->add(&sequencer_status);

        sequence_fileviewer = new FileViewerMenuItem("Sequence");
        menu->add(sequence_fileviewer);
    #endif

    // looper stuff
    #ifdef ENABLE_LOOPER
        menu->add_page("Looper");
        menu->add(mpk49_loop_track.make_menu_items());
        #ifdef ENABLE_DRUM_LOOPER
            menu->add(&drum_looper_status);
            menu->add(&drum_loop_quantizer_setting);
        #endif
    #endif

    #if defined(ENABLE_CRAFTSYNTH_USB) && defined(ENABLE_CRAFTSYNTH_CLOCKTOGGLE)
        menu->add(&craftsynth_clock_toggle);
    #endif

    #ifdef ENABLE_PROFILER
        //DirectNumberControl(const char* label, DataType *target_variable, DataType start_value, DataType min_value, DataType max_value, void (*on_change_handler)(DataType last_value, DataType new_value) = nullptr) 
        DirectNumberControl<uint32_t> *average = new DirectNumberControl<uint32_t>(
            "averages micros per loop", 
            &average_loop_micros, 
            average_loop_micros,
            (uint32_t)0, 
            (uint32_t)(2^64)
        );
        average->readOnly = true;
        menu->add(average);
    #endif

    #endif

    // enable encoder and separate buttons
    #ifdef PIN_BUTTON_A
        pinMode(PIN_BUTTON_A, INPUT_PULLUP);
    #endif
    #ifdef PIN_BUTTON_B
        pinMode(PIN_BUTTON_B, INPUT_PULLUP);
    #endif
    #ifdef PIN_BUTTON_C
        pinMode(PIN_BUTTON_C, INPUT_PULLUP);
    #endif

    Serial.println(F("Exiting setup_menu"));
    Serial_flush();

    /*menu->add(&test_item_1);
    menu->add(&test_item_2);
    menu->add(&test_item_3);*/
}

#endif