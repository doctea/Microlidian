//#include "Config.h"

#include "sequencer/Euclidian.h"
#include "outputs/base_outputs.h"

#ifdef ENABLE_SCREEN
    #include "mymenu.h"
#endif

#include "outputs/output_processor.h"

#ifdef ENABLE_EUCLIDIAN
    EuclidianSequencer *sequencer = nullptr;

    // call this after the menu has already been set up
    void setup_sequencer() {
        sequencer = new EuclidianSequencer(output_processor->nodes);
        sequencer->initialise_patterns();
        sequencer->reset_patterns();
    }

#endif