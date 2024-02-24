#include <Arduino.h>
#include "Config.h"

#include <LinkedList.h>
#include "sequencer/Euclidian.h"

#include "outputs/output.h"

arguments_t initial_arguments[] = {
    { LEN,    4, 1,   DEFAULT_DURATION }, //, TRIGGER_KICK },// get_trigger_for_pitch(GM_NOTE_ELECTRIC_BASS_DRUM) },    // kick
    { LEN,    5, 1,   DEFAULT_DURATION }, //, TRIGGER_SIDESTICK }, //get_trigger_for_pitch(GM_NOTE_SIDE_STICK) },    // stick
    { LEN,    2, 5,   DEFAULT_DURATION }, //, TRIGGER_CLAP }, //get_trigger_for_pitch(GM_NOTE_HAND_CLAP) },    // clap
    { LEN,    3, 1,   DEFAULT_DURATION }, //, TRIGGER_SNARE }, //get_trigger_for_pitch(GM_NOTE_ELECTRIC_SNARE) },   // snare
    { LEN,    3, 3,   DEFAULT_DURATION }, //, TRIGGER_CRASH_1 }, //get_trigger_for_pitch(GM_NOTE_CRASH_CYMBAL_1) },    // crash 1
    { LEN,    7, 1,   DEFAULT_DURATION }, //, TRIGGER_TAMB }, //get_trigger_for_pitch(GM_NOTE_TAMBOURINE) },    // tamb
    { LEN,    9, 1,   DEFAULT_DURATION }, //, TRIGGER_HITOM }, //get_trigger_for_pitch(GM_NOTE_HIGH_TOM) },    // hi tom!
    { LEN/4,  2, 3,   DEFAULT_DURATION }, //, TRIGGER_LOTOM }, //get_trigger_for_pitch(GM_NOTE_LOW_TOM) },    // low tom
    { LEN/2,  2, 3,   DEFAULT_DURATION }, //, TRIGGER_PEDALHAT }, //get_trigger_for_pitch(GM_NOTE_PEDAL_HI_HAT) },    // pedal hat
    { LEN,    4, 3,   DEFAULT_DURATION }, //, TRIGGER_OPENHAT }, //get_trigger_for_pitch(GM_NOTE_OPEN_HI_HAT) },    // open hat
    { LEN,    16, 0,  0                }, //, TRIGGER_CLOSEDHAT }, //get_trigger_for_pitch(GM_NOTE_CLOSED_HI_HAT) }, //DEFAULT_DURATION },   // closed hat
    { LEN*2,  1, 1,   DEFAULT_DURATION }, //, TRIGGER_CRASH_2 }, //get_trigger_for_pitch(GM_NOTE_CRASH_CYMBAL_2) },   // crash 2
    { LEN*2,  1, 5,   DEFAULT_DURATION }, //, TRIGGER_SPLASH }, //get_trigger_for_pitch(GM_NOTE_SPLASH_CYMBAL) },   // splash
    { LEN*2,  1, 9,   DEFAULT_DURATION }, //, TRIGGER_VIBRA }, //get_trigger_for_pitch(GM_NOTE_VIBRA_SLAP) },    // vibra
    { LEN*2,  1, 13,  DEFAULT_DURATION }, //, TRIGGER_RIDE_BELL }, //get_trigger_for_pitch(GM_NOTE_RIDE_BELL) },   // bell
    { LEN*2,  5, 13,  DEFAULT_DURATION }, //, TRIGGER_RIDE_CYM }, //get_trigger_for_pitch(GM_NOTE_RIDE_CYMBAL_1) },   // cymbal
    { LEN,    4, 3,   2 }, //, PATTERN_BASS, 6 },  // bass (neutron) offbeat with 6ie of 6
    { LEN,    4, 3,   1 }, //, PATTERN_MELODY }, //NUM_TRIGGERS+NUM_ENVELOPES },  // melody as above
    { LEN,    4, 1,   4 }, //, PATTERN_PAD_ROOT }, // root pad
    { LEN,    4, 5,   4 } //,   PATTERN_PAD_PITCH); // root pad
};


#if defined(ENABLE_CV_INPUT)
    #include "parameters/Parameter.h"
    #include "parameters/ProxyParameter.h"

    LinkedList<FloatParameter*> *EuclidianSequencer::getParameters() {
        static LinkedList<FloatParameter*> *parameters = nullptr;
        
        if (parameters!=nullptr)
            return parameters;
            
        parameters = new LinkedList<FloatParameter*>();

        // todo: store these in the object, create a page for the local-only ones

        parameters->add(new DataParameter<EuclidianSequencer,float> (
            "Density",
            this,
            &EuclidianSequencer::set_density,
            &EuclidianSequencer::get_density,
            MINIMUM_DENSITY,
            MAXIMUM_DENSITY
        ));

        for (int i = 0 ; i < this->number_patterns ; i++) {
            EuclidianPattern *pattern = (EuclidianPattern *)this->get_pattern(i);
            //LinkedList<FloatParameter*> *pattern_parameters = 
            pattern->getParameters(i);
            //parameter_manager->addParameters(parameters);
            /*for (int i = 0 ; i < pattern_parameters->size() ; i++) {
                parameters->add(pattern_parameters->get(i));
            }*/
        }

        parameter_manager->addParameters(parameters);

        return parameters;
    }
#endif

#if defined(ENABLE_SCREEN) && defined(ENABLE_CV_INPUT)
    #include "mymenu.h"
    #include "mymenu/menuitems_pattern_euclidian.h"

    LinkedList<FloatParameter*> *EuclidianPattern::getParameters(int i) {
        if (parameters!=nullptr)
            return parameters;

        BasePattern::getParameters(i);

        char label[MENU_C_MAX];
        snprintf(label, MENU_C_MAX, "Pattern %i steps", i);
        parameters->add(
            new ProxyParameter<int>(
                label, 
                &this->arguments.steps,
                &this->used_arguments.steps,
                1, 
                this->maximum_steps
            ));

        snprintf(label, MENU_C_MAX, "Pattern %i pulses", i);
        parameters->add(
            new ProxyParameter<int>(
                label,
                &this->arguments.pulses,
                &this->used_arguments.pulses,
                0,
                this->maximum_steps
            )
        );

        snprintf(label, MENU_C_MAX, "Pattern %i rotation", i);
        parameters->add(
            new ProxyParameter<int>(
                label,
                &this->arguments.rotation,
                &this->used_arguments.rotation,
                1,
                this->maximum_steps
            )
        );

        parameter_manager->addParameters(parameters);

        return parameters;
    }

    #include "mymenu_items/ParameterMenuItems_lowmemory.h"
    #include "mymenu_items/ParameterInputMenuItems.h"

    void EuclidianPattern::create_menu_items(Menu *menu, int pattern_index) {
        char label[MENU_C_MAX];
        snprintf(label, MENU_C_MAX, "Pattern %i", pattern_index);
        menu->add_page(label);

        EuclidianPatternControl *epc = new EuclidianPatternControl(label, this);
        menu->add(epc);

        snprintf(label, MENU_C_MAX, "Pattern %i mod", pattern_index);
        menu->add_page(label);

        //snprintf(label, MENU_C_MAX, "Pattern %i")
        LinkedList<FloatParameter*> *parameters = this->getParameters(pattern_index);
        
        /*for (int i = 0 ; i < parameters->size() ; i++) {
            menu->add(parameter_manager->makeMenuItemsForParameter(parameters->get(i)));
        }*/

        // todo: make all this generated by a factory method
        ParameterMenuItemSelector *parameter_selector = new ParameterMenuItemSelector(label, parameters);
        menu->add(parameter_selector);

        SubMenuItemBar *polarity_submenu = new SubMenuItemBar("Polarities", false);
        polarity_submenu->show_header = false;
        polarity_submenu->add(new MenuItem("Polarity"));
        polarity_submenu->add(new ParameterConnectionPolarityTypeSelectorControl("Slot 1", &parameter_selector->actual_controls->parameter, 0));
        polarity_submenu->add(new ParameterConnectionPolarityTypeSelectorControl("Slot 2", &parameter_selector->actual_controls->parameter, 1));
        polarity_submenu->add(new ParameterConnectionPolarityTypeSelectorControl("Slot 3", &parameter_selector->actual_controls->parameter, 2));
        polarity_submenu->add(new MenuItem(""));
        menu->add(polarity_submenu);

        // controls to choose which ParameterInputs to use for each slot
        // then set up a generic submenuitembar to hold the input selectors
        SubMenuItemBar *input_selectors_bar = new SubMenuItemBar("Inputs");
        input_selectors_bar->show_header = false;
        input_selectors_bar->show_sub_headers = false;

        // some spacers so that the input controls align with the corresponding amount controls
        MenuItem *spacer1 = new MenuItem("Inputs");
        spacer1->selectable = false;           
        input_selectors_bar->add(spacer1);

        // make the three source selector controls
        ParameterInputSelectorControl<FloatParameter> *source_selector_1 = new ParameterInputSelectorControl<FloatParameter>(
            "Input 1", 
            &parameter_selector->actual_controls->parameter,
            &FloatParameter::set_slot_0_input,
            &FloatParameter::get_slot_0_input,
            parameter_manager->available_inputs,
            parameter_selector->actual_controls->parameter->connections[0].parameter_input
        );
        source_selector_1->go_back_on_select = true;

        ParameterInputSelectorControl<FloatParameter> *source_selector_2 = new ParameterInputSelectorControl<FloatParameter>(
            "Input 2", 
            &parameter_selector->actual_controls->parameter,
            &FloatParameter::set_slot_1_input,
            &FloatParameter::get_slot_1_input,
            parameter_manager->available_inputs,
            parameter_selector->actual_controls->parameter->connections[1].parameter_input
        );
        source_selector_2->go_back_on_select = true;

        ParameterInputSelectorControl<FloatParameter> *source_selector_3 = new ParameterInputSelectorControl<FloatParameter>(
            "Input 3", 
            &parameter_selector->actual_controls->parameter,
            &FloatParameter::set_slot_2_input,
            &FloatParameter::get_slot_2_input,
            parameter_manager->available_inputs,
            parameter_selector->actual_controls->parameter->connections[2].parameter_input
        );
        source_selector_3->go_back_on_select = true;

        // tell the parameter's connection mappings which screen controls they need to update
        parameter_selector->actual_controls->parameter->link_parameter_input_controls_to_connections(
            parameter_selector->actual_controls->items->get(1),
            parameter_selector->actual_controls->items->get(2),
            parameter_selector->actual_controls->items->get(3),
            source_selector_1,
            source_selector_2,
            source_selector_3
        );

        input_selectors_bar->add(source_selector_1);
        input_selectors_bar->add(source_selector_2);
        input_selectors_bar->add(source_selector_3);

        // empty column at end of bar
        MenuItem *spacer2 = new MenuItem("");
        spacer2->selectable = false;
        input_selectors_bar->add(spacer2);

        menu->add(input_selectors_bar);

        #ifdef SIMPLE_SELECTOR
        OutputSelectorControl<EuclidianPattern> *selector = new OutputSelectorControl<EuclidianPattern>(
            "Output",
            this,
            &EuclidianPattern::set_output,
            &EuclidianPattern::get_output,
            output_processor->nodes,
            this->output
        );
        selector->go_back_on_select = true;
        menu->add(selector);
        #endif
    }

    #include "mymenu.h"
    #include "submenuitem_bar.h"
    #include "mymenu/menuitems_sequencer.h"
    #include "mymenu/menuitems_sequencer_circle.h"
    #include "mymenu/menuitems_outputselectorcontrol.h"

    void EuclidianSequencer::make_menu_items(Menu *menu) {
        menu->add_page("Euclidian", TFT_CYAN);
        for (int i = 0 ; i < this->number_patterns ; i++) {
            char label[MENU_C_MAX];
            snprintf(label, MENU_C_MAX, "Pattern %i", i);
            menu->add(new PatternDisplay(label, this->get_pattern(i)));
            this->get_pattern(i)->colour = menu->get_next_colour();
        }

        menu->add_page("Circle");
        menu->add(new CircleDisplay("Circle", this));

        /*
        // create a dedicated page for the sequencer modulations
        menu->add_page("Sequencer mods");
        LinkedList<FloatParameter*> *parameters = getParameters();
        //parameter_manager->addParameters(parameters);
        for (int i = 0 ; i < parameters->size() ; i++) {
            menu->add(parameters->get(i)->makeControls());
        }*/

        //using option=ObjectSelectorControl<EuclidianPattern,BaseOutput*>::option;
        /*LinkedList<BaseOutput*> *nodes = new LinkedList<BaseOutput*>();
        for (int i = 0 ; i < output_processor.nodes.size() ; i++) {
            nodes->add(output_processor.nodes.get(i));
        }*/

        for (int i = 0 ; i < this->number_patterns ; i++) {
            //Serial.printf("adding controls for pattern %i..\n", i);
            BasePattern *p = (BasePattern *)this->get_pattern(i);

            p->create_menu_items(menu, i);
        }
    }


#endif