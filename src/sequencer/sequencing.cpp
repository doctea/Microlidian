#include "sequencer/Euclidian.h"

#include "mymenu.h"
#include "mymenu/menuitems_sequencer.h"

EuclidianSequencer sequencer = EuclidianSequencer();

void setup_sequencer() {
    sequencer.reset_patterns();

    menu->add_page("Euclidian");
    for (int i = 0 ; i < sequencer.number_patterns ; i++) {
        char label[MENU_C_MAX];
        snprintf(label, MENU_C_MAX, "Pattern %i", i);
        menu->add(new PatternDisplay(label, sequencer.get_pattern(i)));
    }
}