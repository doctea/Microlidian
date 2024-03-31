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
    // todo: improve this bit, maybe name the voltage sources?
    VoltageParameterInput *vpi1 = new VoltageParameterInput((char*)"A", "CV Inputs", parameter_manager->voltage_sources->get(0));
    VoltageParameterInput *vpi2 = new VoltageParameterInput((char*)"B", "CV Inputs", parameter_manager->voltage_sources->get(1));
    VoltageParameterInput *vpi3 = new VoltageParameterInput((char*)"C", "CV Inputs", parameter_manager->voltage_sources->get(2));

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

#ifdef ENABLE_SCREEN
// set up the menus to provide control over the Parameters and ParameterInputs
FLASHMEM void setup_parameter_menu() {
    //Serial.println(F("==== setup_parameter_menu starting ===="));

    //Serial.println(F("Adding ParameterSelectorControls for available_inputs..."));
    // ask ParameterManager to add all the menu items for the ParameterInputs
    parameter_manager->addAllParameterInputMenuItems(menu, true);
    Debug_printf("after addAllParameterInput, free ram is %i\n", rp2040.getFreeHeap());

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