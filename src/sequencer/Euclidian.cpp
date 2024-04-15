#include <Arduino.h>
#include "Config.h"

#include <LinkedList.h>
#include "sequencer/Euclidian.h"

#include "outputs/output.h"

arguments_t initial_arguments[] = {
    { LEN,    4, 1,   DEFAULT_DURATION }, //, TRIGGER_KICK },// get_trigger_for_pitch(GM_NOTE_ELECTRIC_BASS_DRUM) },    // kick
    { LEN,    5, 1,   DEFAULT_DURATION }, //, TRIGGER_SIDESTICK }, //get_trigger_for_pitch(GM_NOTE_SIDE_STICK) },    // stick
    { LEN,    2, 5,   DEFAULT_DURATION }, //, TRIGGER_CLAP }, //get_trigger_for_pitch(GM_NOTE_HAND_CLAP) },    // clap
    { LEN,    3, 5,   DEFAULT_DURATION }, //, TRIGGER_SNARE }, //get_trigger_for_pitch(GM_NOTE_ELECTRIC_SNARE) },   // snare
    { LEN,    3, 3,   DEFAULT_DURATION }, //, TRIGGER_CRASH_1 }, //get_trigger_for_pitch(GM_NOTE_CRASH_CYMBAL_1) },    // crash 1
    { LEN,    7, 1,   DEFAULT_DURATION }, //, TRIGGER_TAMB }, //get_trigger_for_pitch(GM_NOTE_TAMBOURINE) },    // tamb
    { LEN,    9, 1,   DEFAULT_DURATION }, //, TRIGGER_HITOM }, //get_trigger_for_pitch(GM_NOTE_HIGH_TOM) },    // hi tom!
    { LEN/4,  2, 3,   DEFAULT_DURATION }, //, TRIGGER_LOTOM }, //get_trigger_for_pitch(GM_NOTE_LOW_TOM) },    // low tom
    { LEN/2,  2, 3,   DEFAULT_DURATION }, //, TRIGGER_PEDALHAT }, //get_trigger_for_pitch(GM_NOTE_PEDAL_HI_HAT) },    // pedal hat
    { LEN,    4, 3,   DEFAULT_DURATION }, //, TRIGGER_OPENHAT }, //get_trigger_for_pitch(GM_NOTE_OPEN_HI_HAT) },    // open hat
    { LEN,    16, 0,  0                }, //, TRIGGER_CLOSEDHAT }, //get_trigger_for_pitch(GM_NOTE_CLOSED_HI_HAT) }, //DEFAULT_DURATION },   // closed hat
    { LEN*2,  1, 1,   DEFAULT_DURATION_ENVELOPES }, //, TRIGGER_CRASH_2 }, //get_trigger_for_pitch(GM_NOTE_CRASH_CYMBAL_2) },   // crash 2
    { LEN*2,  1, 5,   DEFAULT_DURATION_ENVELOPES }, //, TRIGGER_SPLASH }, //get_trigger_for_pitch(GM_NOTE_SPLASH_CYMBAL) },   // splash
    { LEN*2,  1, 9,   DEFAULT_DURATION_ENVELOPES }, //, TRIGGER_VIBRA }, //get_trigger_for_pitch(GM_NOTE_VIBRA_SLAP) },    // vibra
    { LEN*2,  1, 13,  DEFAULT_DURATION_ENVELOPES }, //, TRIGGER_RIDE_BELL }, //get_trigger_for_pitch(GM_NOTE_RIDE_BELL) },   // bell
    { LEN*2,  5, 13,  DEFAULT_DURATION_ENVELOPES }, //, TRIGGER_RIDE_CYM }, //get_trigger_for_pitch(GM_NOTE_RIDE_CYMBAL_1) },   // cymbal
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

        parameters->add(new ProxyParameter<int_fast8_t>(
            "Mutation amount", 
            &this->mutation_count,
            &this->effective_mutation_count,
            1,
            8
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

        snprintf(label, MENU_C_MAX, "Pattern %i duration", i);
        parameters->add(
            new ProxyParameter<int>(
                label,
                &this->arguments.duration,
                &this->used_arguments.duration,
                1,
                PPQN * 4
            )
        );

        parameter_manager->addParameters(parameters);

        return parameters;
    }

    #include "mymenu_items/ParameterMenuItems_lowmemory.h"

    void EuclidianPattern::create_menu_items(Menu *menu, int pattern_index) {
        char label[MENU_C_MAX];
        snprintf(label, MENU_C_MAX, "Pattern %i", pattern_index);
        menu->add_page(label, this->get_colour());

        EuclidianPatternControl *epc = new EuclidianPatternControl(label, this);
        menu->add(epc);

        snprintf(label, MENU_C_MAX, "Pattern %i mod", pattern_index);
        menu->add_page(label, this->get_colour(), false);

        //snprintf(label, MENU_C_MAX, "Pattern %i")
        LinkedList<FloatParameter*> *parameters = this->getParameters(pattern_index);
        
        /*for (int i = 0 ; i < parameters->size() ; i++) {
            menu->add(parameter_manager->makeMenuItemsForParameter(parameters->get(i)));
        }*/
        create_low_memory_parameter_controls(label, parameters, this->get_colour());

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
    #include "menuitems_object_multitoggle.h"

    class PatternMultiToggleItem : public MultiToggleColourItemClass<BasePattern> {
        public:
        PatternMultiToggleItem(const char *label, BasePattern *target, void(BasePattern::*setter)(bool), bool(BasePattern::*getter)(), bool invert_colours = false) 
            : MultiToggleColourItemClass<BasePattern>(label, target, setter, getter, invert_colours) 
            {}

        BasePattern *last_known_output = nullptr;
        char cached_label[MENU_C_MAX/2];
        virtual const char *get_label() override {
            if (last_known_output!=this->target) {
                snprintf(cached_label, MENU_C_MAX/2, "%s: %s", this->label, this->target->get_output_label());
                //snprintf(cached_label, MENU_C_MAX/2, "%s", this->target->get_output_label());
                last_known_output = this->target;
            }
            //static const char label[MENU_C_MAX];
            //snprintf(label, MENU_C_MAX, "%s: %s", )
            //return (String(this->label) + ": " + this->target->get_output_label()).c_str();
            return cached_label;
        }
    };

    void EuclidianSequencer::make_menu_items(Menu *menu) {
        // add a page for the 'boxed' sequence display of all tracks
        menu->add_page("Euclidian", TFT_CYAN);
        for (int i = 0 ; i < this->number_patterns ; i++) {
            char label[MENU_C_MAX];
            snprintf(label, MENU_C_MAX, "Pattern %i", i);
            menu->add(new PatternDisplay(label, this->get_pattern(i)));
            this->get_pattern(i)->colour = menu->get_next_colour();
        }

        // add a page for the circle display that shows all tracks simultaneously
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

        // single page for multitoggle to lock patterns
        menu->add_page("Pattern locks", C_WHITE, false);
        ObjectMultiToggleColumnControl *toggle = new ObjectMultiToggleColumnControl("Allow changes", true);
        for (unsigned int i = 0 ; i < this->number_patterns ; i++) {
            BasePattern *p = (BasePattern *)this->get_pattern(i);

            PatternMultiToggleItem *option = new PatternMultiToggleItem(
                (new String(String("Pattern ") + String(i)))->c_str(),
                //p->get_output_label(),  // todo: make class auto-update 
                p,
                &BasePattern::set_locked,
                &BasePattern::is_locked,
                true
            );
            toggle->addItem(option);
        }
        menu->add(toggle);

        // ask each pattern to add their menu pages
        for (int i = 0 ; i < this->number_patterns ; i++) {
            //Serial.printf("adding controls for pattern %i..\n", i);
            BasePattern *p = (BasePattern *)this->get_pattern(i);

            p->create_menu_items(menu, i);
        }
    }


#endif