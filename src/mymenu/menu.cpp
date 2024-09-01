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
        pushButtonA.interval(5);
        pushButtonA.setPressedState(button_high_state);
    #endif
    #ifdef PIN_BUTTON_B
        pushButtonB.attach(PIN_BUTTON_B, INPUT);
        pushButtonB.interval(5);
        pushButtonB.setPressedState(button_high_state);
    #endif
    #ifdef PIN_BUTTON_C
        pushButtonB.attach(PIN_BUTTON_C, INPUT);
        pushButtonB.interval(5);
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
    /*#ifdef ENABLE_SCALES
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
    #endif*/

    //menu->add(&test_item_1);

    // this is where euclidian menu was originally set up
    
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