#include "Config.h"
//#include "storage.h"

#include "debug.h"

#include <Arduino.h>

#include "sequencer/sequencing.h"
#include "sequencer/Euclidian.h"
#include "outputs/output_processor.h"

#ifdef ENABLE_SCREEN

/*#include <Adafruit_GFX_Buffer.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>*/

#include "menu_io.h"

#include "mymenu.h"

#include "submenuitem_bar.h"

#include "mymenu/menu_bpm.h"
#include "mymenu/menu_clock_source.h"

#include "mymenu/menuitems_scale.h"

#include "LinkedList.h"
#include "parameters/Parameter.h"

//DisplayTranslator *tft;
DisplayTranslator_Configured *tft = nullptr;
Menu *menu = nullptr; // = Menu();

#ifdef ENCODER_KNOB_L
    //Encoder knob(ENCODER_KNOB_L, ENCODER_KNOB_R);
    Encoder *knob = nullptr; // earlephilhower core trashes interrupts after static initialisation, so set up Encoder in setup() instead now
#endif
#ifdef PIN_BUTTON_A
    Button pushButtonA = Button(); // 10ms debounce
    //extern Bounce pushButtonA;
#endif
#ifdef PIN_BUTTON_B
    Button pushButtonB = Button(); // 10ms debounce
    //extern Bounce pushButtonB; 
#endif
#ifdef PIN_BUTTON_C
    Button pushButtonC = Button(); // 10ms debounce
    //extern Bounce pushButtonC;
#endif

LoopMarkerPanel top_loop_marker_panel = LoopMarkerPanel(LOOP_LENGTH_TICKS, PPQN, BEATS_PER_BAR, BARS_PER_PHRASE);

BPMPositionIndicator posbar = BPMPositionIndicator();
ClockSourceSelectorControl clock_source_selector = ClockSourceSelectorControl("Clock source", clock_mode);

DisplayTranslator_Configured displaytranslator = DisplayTranslator_Configured();

//float bpm_selector_values[] = { 60, 90, 120, 150, 180, 500, 1000, 2000, 3000 };

/*#ifndef GDB_DEBUG
FLASHMEM 
#endif*/
void setup_menu(bool button_high_state = HIGH) {
    Debug_println(F("Starting setup_menu()..")); Serial_flush();

    #ifdef ENCODER_KNOB_L
        knob = new Encoder(ENCODER_KNOB_L, ENCODER_KNOB_R);
    #endif

    #ifdef PIN_BUTTON_A
        pushButtonA.attach(PIN_BUTTON_A, INPUT);
        pushButtonA.interval(10);
        pushButtonA.setPressedState(button_high_state);
    #endif
    #ifdef PIN_BUTTON_B
        pushButtonB.attach(PIN_BUTTON_B, INPUT);
        pushButtonB.interval(10);
        pushButtonB.setPressedState(button_high_state);
    #endif
    #ifdef PIN_BUTTON_C
        pushButtonB.attach(PIN_BUTTON_C, INPUT);
        pushButtonB.interval(10);
        pushButtonB.setPressedState(button_high_state);
    #endif

    tft = &displaytranslator; 
    tft->init();

    //tft->set_default_textsize(1); // needs some work to make this work properly...

    for (int x = 0 ; x < tft->width() ; x+=3) {
        for (int y = 0 ; y < tft->height() ; y+=3) {
            tft->drawLine(0,0,x,y,random(65535));
        }        
    }

    //delay(50);
    //Serial.println(F("Finished  constructor"));
    Serial_flush();
    Debug_println(F("Creating Menu object..")); Serial_flush();
    menu = new Menu(tft, true);
    Debug_println(F("Created Menu object")); Serial_flush();

    menu->set_screen_height_cutoff(0.5f);

    menu->set_messages_log(messages_log);

    menu->add_pinned(&top_loop_marker_panel);  // pinned position indicator
    menu->add(&posbar);                        // bpm and position indicator
    menu->add(&clock_source_selector);         // midi clock source (internal or from PC USB)

    // add start/stop/continue bar
    SubMenuItemBar *project_startstop = new SubMenuItemBar("Transport", false);
    project_startstop->add(new ActionItem("Start",    clock_start));
    project_startstop->add(new ActionItem("Stop",     clock_stop));
    project_startstop->add(new ActionItem("Continue", clock_continue));
    project_startstop->add(new ActionFeedbackItem("Restart", (ActionFeedbackItem::setter_def_2)set_restart_on_next_bar_on, is_restart_on_next_bar, "Restarting..", "Restart"));
    menu->add(project_startstop);

    // debug bpm selector
    /*SelectorControl<float> *bpm_selector = new SelectorControl<float>("BPM");
    bpm_selector->available_values = bpm_selector_values;
    bpm_selector->num_values = sizeof(bpm_selector_values)/sizeof(float);
    bpm_selector->f_setter = set_bpm;
    bpm_selector->f_getter = get_bpm;
    menu->add(bpm_selector);*/

    // debug scales page
    #ifdef ENABLE_SCALES
        //menu->add_page("Scales");
        //menu->add(new ScaleMenuItem("Scales"));

        menu->add_page("Bass Quantiser");
        menu->add(new ObjectScaleMenuItem<MIDINoteTriggerCountOutput>("Bass scale", 
            (MIDINoteTriggerCountOutput*)output_processor->nodes->get(output_processor->nodes->size()-1),
            &MIDINoteTriggerCountOutput::set_scale_number, 
            &MIDINoteTriggerCountOutput::get_scale_number, 
            &MIDINoteTriggerCountOutput::set_scale_root, 
            &MIDINoteTriggerCountOutput::get_scale_root,
            true
        ));
    #endif

    //menu->add(&test_item_1);

    #ifdef ENABLE_EUCLIDIAN
        menu->add_page("Mutation");
        SubMenuItemColumns *submenu = new SubMenuItemColumns("Euclidian Mutations", 3);
        submenu->add(new ObjectNumberControl<EuclidianSequencer,float>("Density", &sequencer, &EuclidianSequencer::set_density,        &EuclidianSequencer::get_density, nullptr, MINIMUM_DENSITY, MAXIMUM_DENSITY));
        submenu->add(new ObjectToggleControl<EuclidianSequencer>("Mutate", &sequencer, &EuclidianSequencer::set_mutated_enabled,       &EuclidianSequencer::is_mutate_enabled));
        submenu->add(new ObjectToggleControl<EuclidianSequencer>("Reset", &sequencer,  &EuclidianSequencer::set_reset_before_mutate,   &EuclidianSequencer::should_reset_before_mutate));
        submenu->add(new ObjectToggleControl<EuclidianSequencer>("Add phrase", &sequencer, &EuclidianSequencer::set_add_phrase_enabled,&EuclidianSequencer::is_add_phrase_enabled));
        submenu->add(new ObjectToggleControl<EuclidianSequencer>("Fills", &sequencer,  &EuclidianSequencer::set_fills_enabled,         &EuclidianSequencer::is_fills_enabled));
        submenu->add(new ObjectNumberControl<EuclidianSequencer,int>("Seed", &sequencer, &EuclidianSequencer::set_euclidian_seed,      &EuclidianSequencer::get_euclidian_seed));
        menu->add(submenu);

        #ifdef ENABLE_CV_INPUT
        // add the sequencer modulation controls to this page
        LinkedList<FloatParameter*> *sequencer_parameters = sequencer.getParameters();
        for (int i = 0 ; i < sequencer_parameters->size() ; i++) {
            menu->add(sequencer_parameters->get(i)->makeControls());
        }
        #endif
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

    Debug_printf(F("Exiting setup_menu"));
    Serial_flush();

    /*menu->add(&test_item_1);
    menu->add(&test_item_2);
    menu->add(&test_item_3);*/
}

#endif