#include "Config.h"

#include "sequencer/Euclidian.h"

#include "mymenu.h"
#include "submenuitem_bar.h"
#include "mymenu/menuitems_sequencer.h"
#include "mymenu/menuitems_sequencer_circle.h"
#include "mymenu/menuitems_outputselectorcontrol.h"

#include "outputs/output.h"

#ifdef ENABLE_EUCLIDIAN
    EuclidianSequencer sequencer = EuclidianSequencer();

    extern MIDIOutputProcessor output_processer;

    // call this after the menu has already been set up
    void setup_sequencer() {
        sequencer.initialise_patterns();
    }

    void setup_sequencer_menu() {

        menu->add_page("Euclidian", TFT_CYAN);
        for (int i = 0 ; i < sequencer.number_patterns ; i++) {
            char label[MENU_C_MAX];
            snprintf(label, MENU_C_MAX, "Pattern %i", i);
            menu->add(new PatternDisplay(label, sequencer.get_pattern(i)));
            sequencer.get_pattern(i)->colour = menu->get_next_colour();
        }

        menu->add_page("Circle");
        menu->add(new CircleDisplay("Circle", &sequencer));

        //using option=ObjectSelectorControl<EuclidianPattern,BaseOutput*>::option;
        /*LinkedList<BaseOutput*> *nodes = new LinkedList<BaseOutput*>();
        for (int i = 0 ; i < output_processer.nodes.size() ; i++) {
            nodes->add(output_processer.nodes.get(i));
        }*/

        for (int i = 0 ; i < sequencer.number_patterns ; i++) {
            //Serial.printf("adding controls for pattern %i..\n", i);
            char label[MENU_C_MAX];
            snprintf(label, MENU_C_MAX, "Pattern %i", i);
            menu->add_page(label);
            EuclidianPattern *p = (EuclidianPattern *)sequencer.get_pattern(i);

            menu->add(new PatternDisplay(label, p));

            OutputSelectorControl<EuclidianPattern> *selector = new OutputSelectorControl<EuclidianPattern>(
                "Output",
                p,
                &EuclidianPattern::set_output,
                &EuclidianPattern::get_output,
                &output_processer.nodes,
                p->output
            );
            selector->go_back_on_select = true;
            menu->add(selector);

            //menu->add(new ObjectToggleControl<EuclidianPattern> ("Locked", p, &EuclidianPattern::set_locked, &EuclidianPattern::is_locked));

            SubMenuItemBar *bar = new SubMenuItemBar("Arguments");
            //Menu *bar = menu;
            bar->add(new ObjectNumberControl<EuclidianPattern,byte> ("Steps",    p, &EuclidianPattern::set_steps,      &EuclidianPattern::get_steps,    nullptr, 1, STEPS_PER_BAR, true, true));
            bar->add(new ObjectNumberControl<EuclidianPattern,byte> ("Pulses",   p, &EuclidianPattern::set_pulses,     &EuclidianPattern::get_pulses,   nullptr, 1, STEPS_PER_BAR, true, true));
            bar->add(new ObjectNumberControl<EuclidianPattern,byte> ("Rotation", p, &EuclidianPattern::set_rotation,   &EuclidianPattern::get_rotation, nullptr, 1, STEPS_PER_BAR, true, true));
            menu->add(bar);

            //menu->add(new ObjectNumberControl<EuclidianPattern,byte> ("Duration", p, &EuclidianPattern::set_duration,   &EuclidianPattern::get_duration, nullptr, 1, STEPS_PER_BAR, true, true));
            //menu->debug = true;

            menu->add(new ObjectActionConfirmItem<EuclidianPattern> ("Store as default", p, &EuclidianPattern::store_current_arguments_as_default));
        }

        menu->select_page(0);
    }
#endif