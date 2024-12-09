#include "Config.h"

#include "clock.h"
#include "bpm.h"

#ifdef ENABLE_CV_INPUT

#include "cv_input.h"

#include "ParameterManager.h"
#include "colours.h"
#include "submenuitem.h"

#include "devices/ADCPimoroni24v.h"

#include "parameter_inputs/VirtualParameterInput.h"

//#define LOOP_LENGTH_TICKS   (PPQN*BEATS_PER_BAR*BARS_PER_PHRASE)

#include "sequencer/sequencing.h"

#include "Wire.h"

//#include "behaviours/behaviour_base.h"
//#include "behaviours/behaviour_craftsynth.h"

bool cv_input_enabled = true;

ParameterManager *parameter_manager = new ParameterManager(LOOP_LENGTH_TICKS);

// initialise the voltage-reading hardware/libraries and the ParameterManager
FLASHMEM
void setup_cv_input() {
    //Serial.println("setup_cv_input...");
    tft_print("...setup_cv_input...\n");

    parameter_manager->init();

    Wire.begin();

    #ifdef ENABLE_CV_INPUT
        parameter_manager->addADCDevice(new ADCPimoroni24v(ENABLE_CV_INPUT, &Wire, 5.0)); //, 5.0)); //, 2, MAX_INPUT_VOLTAGE_24V));
    #endif

    parameter_manager->auto_init_devices();

    tft_print("..done setup_cv_input\n");
}

// initialise the input voltage ParameterInputs that can be mapped to Parameters
FLASHMEM 
void setup_parameter_inputs() {
    //parameter_manager = new ParameterManager();
    // add the available parameters to a list used globally and later passed to each selector menuitem
    //Serial.println(F("==== begin setup_parameter_inputs ====")); Serial_flush();
    tft_print("..setup_parameter_inputs...");

    // initialise the voltage source inputs
    // NOTE: for some reason inputs 1 + 2 (B + C) seem to be swapped on this revision of the hardware; so swap them here in software.
    VoltageParameterInput *vpi1 = new VoltageParameterInput((char*)"A", "CV Inputs", parameter_manager->voltage_sources->get(0));
    VoltageParameterInput *vpi2 = new VoltageParameterInput((char*)"B", "CV Inputs", parameter_manager->voltage_sources->get(2));
    VoltageParameterInput *vpi3 = new VoltageParameterInput((char*)"C", "CV Inputs", parameter_manager->voltage_sources->get(1));
    
    //vpi3->input_type = UNIPOLAR;
    // todo: set up 1v/oct inputs to map to MIDI source_ids...

    // tell the parameter manager about them
    parameter_manager->addInput(vpi1);
    parameter_manager->addInput(vpi2);
    parameter_manager->addInput(vpi3);

    VirtualParameterInput *virtpi1 = new VirtualParameterInput((char*)"LFO sync", "LFOs", LFO_LOCKED);
    VirtualParameterInput *virtpi2 = new VirtualParameterInput((char*)"LFO free", "LFOs", LFO_FREE);
    VirtualParameterInput *virtpi3 = new VirtualParameterInput((char*)"Random",   "LFOs", RAND);
    parameter_manager->addInput(virtpi1);
    parameter_manager->addInput(virtpi2);
    parameter_manager->addInput(virtpi3);

    //Serial.println("about to do setDefaultParameterConnections().."); Serial.flush();
    parameter_manager->setDefaultParameterConnections();
    //"Serial.println("just did do setDefaultParameterConnections().."); Serial.flush();

    tft_print("Finished setup_parameter_inputs()\n");
}


#define NUM_MIDI_CC_PARAMETERS 6
FloatParameter *midi_cc_parameters[NUM_MIDI_CC_PARAMETERS];

// todo: configure the CCs to be compatible with the CCs of the midimuso by default etc
FLASHMEM
void setup_parameter_outputs(IMIDICCTarget *wrapper) {
    int c = 0;
    FloatParameter *p = midi_cc_parameters[c++] = parameter_manager->addParameter(new MIDICCParameter<>("A", wrapper, 0, 1, true, true));
    p->connect_input(0, 1.0f); p->connect_input(1, 0.0f); p->connect_input(2, 0.0f);

    p = midi_cc_parameters[c++] = parameter_manager->addParameter(new MIDICCParameter<>("B", wrapper,    1, 1, true, true));
    p->connect_input(0, 0.0f); p->connect_input(1, 1.0f); p->connect_input(2, 0.0f);

    p = midi_cc_parameters[c++] = parameter_manager->addParameter(new MIDICCParameter<>("C", wrapper,    7, 1, true, true));
    p->connect_input(0, 0.0f); p->connect_input(1, 0.0f); p->connect_input(2, 1.0f);

    p = midi_cc_parameters[c++] = parameter_manager->addParameter(new MIDICCParameter<>("Mix1", wrapper, 11, 1, true, true));
    p->connect_input(0, 1.0f); p->connect_input(1, 1.0f); p->connect_input(2, 0.0f);

    p = midi_cc_parameters[c++] = parameter_manager->addParameter(new MIDICCParameter<>("Mix2", wrapper, 74, 1, true, true));
    p->connect_input(0, 0.0f); p->connect_input(1, 1.0f); p->connect_input(2, 1.0f);

    p = midi_cc_parameters[c++] = parameter_manager->addParameter(new MIDICCParameter<>("Mix3", wrapper, 76, 1, true, true));
    p->connect_input(0, 1.0f); p->connect_input(1, 0.0f); p->connect_input(2, 1.0f);   
}

#ifdef ENABLE_SCREEN
    #include "mymenu_items/ParameterMenuItems_lowmemory.h"
    FLASHMEM
    void create_parameter_output_menu_items() {
        char label[MENU_C_MAX];
        for (int i = 0 ; i < NUM_MIDI_CC_PARAMETERS ; i++) {
            snprintf(label, MENU_C_MAX, "CV-to-MIDI: %s", midi_cc_parameters[i]->label);
            menu->add_page(label, C_WHITE);

            /*
            // todo: CC+channel selectors now moved to MIDICCParameter#addCustomTypeControls
            snprintf(label, MENU_C_MAX, "Settings");
            SubMenuItem *bar = new SubMenuItemBar(label, true, false);

            snprintf(label, MENU_C_MAX, "Output CC");
            bar->add(new DirectNumberControl<byte>(
                label, 
                &midi_cc_parameters[i].cc_number,
                midi_cc_parameters[i].cc_number,
                0,
                MIDI_MAX_VELOCITY
            ));

            snprintf(label, MENU_C_MAX, "Output MIDI Channel");
            bar->add(new DirectNumberControl<byte>(
                label, 
                &midi_cc_parameters[i].channel,
                midi_cc_parameters[i].channel,
                1,
                16
            ));

            menu->add(bar);*/
            
            //menu->add(midi_cc_parameters[i].makeControls());
            //use lowmemory controls instead of a full instance of each

            // add a separator bar for the parameter; don't think we actually want this though...
            //menu->add(new SeparatorMenuItem(midi_cc_parameters[i].label));            
            create_low_memory_parameter_controls(midi_cc_parameters[i]->label, midi_cc_parameters[i]);
        }
    }
#endif

#ifdef ENABLE_SCREEN
// set up the menus to provide control over the Parameters and ParameterInputs
FLASHMEM void setup_parameter_menu() {
    //Serial.println(F("==== setup_parameter_menu starting ===="));

    //Serial.println(F("Adding ParameterSelectorControls for available_inputs..."));
    // ask ParameterManager to add all the menu items for the ParameterInputs
    parameter_manager->addAllParameterInputMenuItems(menu, true);
    Debug_printf("after addAllParameterInput, free ram is %i\n", rp2040.getFreeHeap());

    // ask ParameterManager to build overviews for the ParameterInput groups; currently this must be done after those controls have already been created.
    parameter_manager->addAllParameterInputOverviews(menu);
    Debug_printf("after addAllParameterInputOverviews, free ram is %i\n", rp2040.getFreeHeap());

    //parameter_manager->addAllVoltageSourceMenuItems(menu);
    //Serial.println("About to addAllVoltageSourceCalibrationMenuItems().."); Serial.flush();
    parameter_manager->addAllVoltageSourceCalibrationMenuItems(menu, true);
    Debug_printf("after addAllVoltageSourceCalibrationMenuItems, free ram is %i\n", rp2040.getFreeHeap());

    //Serial.println("About to addAllParameterMenuItems().."); Serial.flush();
    //#ifdef ENABLE_PARAMETER_MAPPING
        //menu->add_page("Parameters");
        // TODO: add the 'global' parameters, eg Density
        //parameter_manager->addAllParameterMenuItems(menu);
        Debug_printf("after addAllParameterMenuItems, free ram is %i\n", rp2040.getFreeHeap());
    //#endif

    parameter_manager->setDefaultParameterConnections();

    //DirectNumberControl<int> *mixer_profile = new DirectNumberControl<int>("Mixer profiling", &parameter_manager->profile_update_mixers, parameter_manager->profile_update_mixers, (int)0, (int)1000000, nullptr);
    //menu->add(mixer_profile);

    //Serial.println(F("setup_parameter_menu done =================="));
}
#endif

#endif