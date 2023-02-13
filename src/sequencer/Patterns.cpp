#include "sequencer/Patterns.h"

#include "outputs/output.h"

void SimplePattern::trigger_on_for_step(int step) {
    if (this->output!=nullptr) 
        this->output->receive_event(1,0,0);
}
void SimplePattern::trigger_off_for_step(int step) {
    if (this->output!=nullptr) 
        this->output->receive_event(0,1,0);
};
