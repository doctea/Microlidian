#include <Arduino.h>

#include <LinkedList.h>

#include "sequencer/Euclidian.h"
#include "parameters/Parameter.h"

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
    LinkedList<FloatParameter*> *EuclidianSequencer::getParameters() {
        LinkedList<FloatParameter*> *parameters = new LinkedList<FloatParameter*>();

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
            char label[MENU_C_MAX];
            snprintf(label, MENU_C_MAX, "Pattern %i steps", i);
            parameters->add(
                new DataParameter<EuclidianPattern,byte>(
                    label, 
                    pattern, 
                    &EuclidianPattern::set_steps, 
                    &EuclidianPattern::get_steps, 
                    1, 
                    pattern->maximum_steps
                ));

            snprintf(label, MENU_C_MAX, "Pattern %i pulses", i);
            parameters->add(
                new DataParameter<EuclidianPattern,byte>(
                    label,
                    pattern,
                    &EuclidianPattern::set_pulses,
                    &EuclidianPattern::get_pulses,
                    0,
                    pattern->maximum_steps
                )
            );

            snprintf(label, MENU_C_MAX, "Pattern %i rotation", i);
            parameters->add(
                new DataParameter<EuclidianPattern,byte>(
                    label,
                    pattern,
                    &EuclidianPattern::set_rotation,
                    &EuclidianPattern::get_rotation,
                    0,
                    pattern->maximum_steps
                )
            );
        }
        return parameters;
    }
#endif

#ifdef ENABLE_SCREEN
    #include "mymenu.h"
    #include "mymenu/menuitems_pattern_euclidian.h"

    void EuclidianPattern::create_menu_items(Menu *menu, int pattern_index) {
        char label[MENU_C_MAX];
        snprintf(label, MENU_C_MAX, "Pattern %i", pattern_index);
        menu->add_page(label);

        EuclidianPatternControl *epc = new EuclidianPatternControl(label, this);
        menu->add(epc);
        
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
#endif