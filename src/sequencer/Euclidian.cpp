#include <Arduino.h>

#include <LinkedList.h>

#include "sequencer/Euclidian.h"
#include "parameters/Parameter.h"

#include "outputs/output.h"

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

        //EuclidianPattern *p = (EuclidianPattern *)sequencer.get_pattern(pattern_index);
        EuclidianPatternControl *epc = new EuclidianPatternControl(label, this);
        menu->add(epc);
    }
#endif