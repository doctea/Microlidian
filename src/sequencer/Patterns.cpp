#include "sequencer/Patterns.h"

#include "outputs/output.h"

void SimplePattern::trigger_on_for_step(int step) {
    this->triggered_on_step = step; // % (ticks_per_step*steps);
    this->current_duration = this->get_tick_duration();

    if (this->output!=nullptr) {
        this->output->receive_event(1,0,0);
        note_held = true;
    }
}
void SimplePattern::trigger_off_for_step(int step) {
    this->triggered_on_step = -1;
    if (this->output!=nullptr) {
        this->output->receive_event(0,1,0);
        this->output->process();
        note_held = false;
    }
};

LinkedList<FloatParameter*> *BasePattern::getParameters(int i) {
    if (this->parameters==nullptr)
        this->parameters = new LinkedList<FloatParameter*>();
    return this->parameters;
}

void BasePattern::create_menu_items(Menu *menu, int pattern_index) {
    // nothing to be done for base pattern case
    //pattern_index += 1;
}

/*void SimplePattern::create_menu_items(Menu *menu, int pattern_index) {
    // nothing to be done for simple pattern case
}
*/