#include "sequencer/Sequencer.h"
#include "sequencer/Patterns.h"

//#include "outputs/output.h"

void BaseSequencer::configure_pattern_output(int index, BaseOutput *output) {
    SimplePattern *p = this->get_pattern(index);
    p->set_output(output);
}