#include "sequencer/Sequencer.h"
#include "sequencer/Patterns.h"

#include "debug.h"

//#include "outputs/output.h"

void BaseSequencer::configure_pattern_output(int index, BaseOutput *output) {
    if (index >= this->number_patterns) {
        char message[MAX_MESSAGE_LOG*2];
        snprintf(message, MAX_MESSAGE_LOG*2, "Attempted to configure pattern with invalid index %i!", index);
        messages_log_add(message);
        return;
    }
    SimplePattern *p = this->get_pattern(index);
    p->set_output(output);
}