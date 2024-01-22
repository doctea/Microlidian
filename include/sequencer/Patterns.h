#ifndef PATTERNS__INCLUDED
#define PATTERNS__INCLUDED

#include <Arduino.h>

#include "Config.h"

#include "clock.h"
#include "midi_helpers.h"

#ifdef ENABLE_SCREEN
    #include "menu.h"
#endif

#ifdef ENABLE_CV_INPUT
    #include "parameters/Parameter.h"
#endif

#define DEFAULT_VELOCITY    MIDI_MAX_VELOCITY

#define MAX_STEPS 32

class BaseOutput;

class BasePattern {
    public:

    byte steps = MAX_STEPS;
    int steps_per_beat = STEPS_PER_BEAT;
    int ticks_per_step = PPQN / steps_per_beat;            // todo: calculate this from desired pattern length in bars, PPQN and steps
    bool note_held = false;

    #ifdef ENABLE_SCREEN
        int16_t colour = C_WHITE;
    #endif

    virtual const char *get_summary() {
        return "??";
    }

    // todo: ability to pass in step, offset, and bar number, like we have for the current euclidian...?
    //          or, tbf, we can derive this from the 'tick'
    virtual bool query_note_on_for_tick(unsigned int tick) = 0;
    virtual bool query_note_off_for_tick(unsigned int tick) = 0;

    virtual void set_event_for_tick(unsigned int tick, short note = 0, short velocity = 127, short channel = 0) = 0;
    virtual void unset_event_for_tick(unsigned int tick) = 0;

    virtual void process_step(int step) = 0;
    virtual void process_step_end(int step) = 0;
    virtual void process_tick(int tick) = 0;

    //virtual bool query_note_on_for_step(int step) = 0;
    //virtual bool query_note_off_for_step(int step) = 0;

    virtual bool query_note_on_for_step(int step) {
        return this->query_note_on_for_tick(step * ticks_per_step);
    }
    virtual bool query_note_off_for_step(int step) {
        return this->query_note_off_for_tick(step * ticks_per_step);
    }

    // duration of the note about to be played
    virtual int get_tick_duration() {
        return PPQN;
    }

    virtual void set_steps(byte steps) {
        this->steps = steps;
    }
    virtual byte get_steps() {
        return this->steps;
    }

    #ifdef ENABLE_SCREEN
        #ifdef ENABLE_CV_INPUT
            LinkedList<FloatParameter*> *parameters = nullptr;
            virtual LinkedList<FloatParameter*> *getParameters(int i);
        #endif
        virtual void create_menu_items(Menu *menu, int index);
    #endif
};

class SimplePattern : public BasePattern {
    public:

    struct event {
        byte note = NOTE_OFF;
        byte velocity = DEFAULT_VELOCITY;
        byte channel = 0;
    };

    int triggered_on_step = -1;
    int current_duration = PPQN;

    BaseOutput *output = nullptr;
    event *events = nullptr;

    SimplePattern() : BasePattern() {
        this->events = (event*)calloc(sizeof(event), steps);
    }

    /*const char *get_label() {

    }*/

    virtual void set_output(BaseOutput *output) {
        this->output = output;
    }
    virtual BaseOutput *get_output() {
        return this->output;
    }

    virtual unsigned int get_step_for_tick(unsigned int tick) {
        return (tick / this->ticks_per_step) % this->get_steps();
    }

    virtual void set_event_for_tick(unsigned int tick, short note = 0, short velocity = DEFAULT_VELOCITY, short channel = 0) override {
        short step = get_step_for_tick(tick);
        this->events[step].note = note;
        this->events[step].velocity = velocity;
        this->events[step].channel = channel;
    }
    virtual void unset_event_for_tick(unsigned int tick) override {
        short step = get_step_for_tick(tick);
        this->events[step].velocity = 0;
        //this->events[step].note = NOTE_OFF;
        //this->events[step].channel = channel;
    }

    virtual bool query_note_on_for_tick(unsigned int tick) override {
        short step = get_step_for_tick(tick);
        return (this->events[step].velocity>0);
    }
    virtual bool query_note_off_for_tick(unsigned int tick) override {
        short step = get_step_for_tick(tick);
        return (this->events[step].velocity==0);
    }

    virtual void process_step(int step) override {
        //Serial.printf("process_step(%i)\t");
        /*if (this->query_note_off_for_step((step-1) % this->get_steps()) && this->note_held) {
            //Serial.printf("%i: note off for step!");
            this->trigger_off_for_step(step);
        }*/
        if (this->query_note_on_for_step(step)) {
            //Serial.printf("query_note_on_for_step!,");
            this->trigger_on_for_step(step);
        }
        //Serial.println();
    };
    virtual void process_step_end(int step) override {
        if (this->query_note_off_for_step((step+1) % this->get_steps()) && this->note_held) {
            //Serial.printf("%i: note off for step!");
            this->trigger_off_for_step(step);
        }
    }
    virtual void process_tick(int ticks) override { 
        // check if note is held and duration has passed...
        int step = (ticks / ticks_per_step); // % steps;
        //ticks = ticks % (ticks_per_step * steps);

        if ((triggered_on_step * ticks_per_step) + this->current_duration <= ticks || ticks < triggered_on_step * ticks_per_step) {
            this->trigger_off_for_step(step);
        }
    }

    virtual void trigger_on_for_step(int step);
    virtual void trigger_off_for_step(int step);

    virtual void restore_default_arguments() {}
    virtual void store_current_arguments_as_default() {}

    /*#ifdef ENABLE_SCREEN
        virtual void create_menu_items(Menu *menu, int index) override;
    #endif*/

};


#endif