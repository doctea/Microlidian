#include "Config.h"

#include "sequencer/Euclidian.h"
#include "outputs/output.h"

#ifdef ENABLE_SCREEN
    #include "mymenu.h"
    #include "submenuitem_bar.h"
    #include "mymenu/menuitems_sequencer.h"
    #include "mymenu/menuitems_sequencer_circle.h"
    #include "mymenu/menuitems_outputselectorcontrol.h"
#endif


#ifdef ENABLE_EUCLIDIAN
    EuclidianSequencer sequencer = EuclidianSequencer();

    // call this after the menu has already been set up
    void setup_sequencer() {
        sequencer.initialise_patterns();
    }

    #ifdef ENABLE_SCREEN
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
            for (int i = 0 ; i < output_processor.nodes.size() ; i++) {
                nodes->add(output_processor.nodes.get(i));
            }*/

            for (int i = 0 ; i < sequencer.number_patterns ; i++) {
                //Serial.printf("adding controls for pattern %i..\n", i);
                EuclidianPattern *p = (EuclidianPattern *)sequencer.get_pattern(i);

                p->create_menu_items(menu, i);
            }

            menu->select_page(0);
        }
    #endif
#endif