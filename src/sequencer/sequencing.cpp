#include "Config.h"

#include "sequencer/Euclidian.h"
#include "outputs/output.h"

#ifdef ENABLE_SCREEN
    #include "mymenu.h"
#endif


#ifdef ENABLE_EUCLIDIAN
    EuclidianSequencer sequencer = EuclidianSequencer();

    // call this after the menu has already been set up
    void setup_sequencer() {
        sequencer.initialise_patterns();
    }

    #ifdef ENABLE_SCREEN
        void setup_sequencer_menu() {

            sequencer.make_menu_items(menu);

            menu->select_page(0);
        }
    #endif
#endif