#ifndef PATTERNS__INCLUDED
#define PATTERNS__INCLUDED

#include <Arduino.h>

#include "clock.h"

#ifdef ENABLE_SCREEN
    #include "menu.h"
#endif

#define NOTE_OFF -1
#define DEFAULT_VELOCITY 127
//#define PPQN    24

class BaseOutput;

class BasePattern {
    public:

    byte steps = 32;
    int steps_per_beat = STEPS_PER_BEAT;
    int ticks_per_step = PPQN / steps_per_beat;            // todo: calculate this from desired pattern length in bars, PPQN and steps
    bool note_held = false;

    int16_t colour = C_WHITE;

    virtual char *get_summary() {
        return "??";
    }

    // todo: ability to pass in step, offset, and bar number, like we have for the current euclidian...?
    //          or, tbf, we can derive this from the 'tick'
    virtual bool query_note_on_for_tick(unsigned int tick) = 0;
    virtual bool query_note_off_for_tick(unsigned int tick) = 0;

    virtual void set_event_for_tick(unsigned int tick, short note = 0, short velocity = 127, short channel = 0) = 0;
    virtual void unset_event_for_tick(unsigned int tick) = 0;

    virtual void process_step(int step) = 0;

    //virtual bool query_note_on_for_step(int step) = 0;
    //virtual bool query_note_off_for_step(int step) = 0;

    virtual bool query_note_on_for_step(int step) {
        return this->query_note_on_for_tick(step * ticks_per_step);
    }
    virtual bool query_note_off_for_step(int step) {
        return this->query_note_off_for_tick(step * ticks_per_step);
    }

    virtual void set_steps(byte steps) {
        this->steps = steps;
    }
    virtual byte get_steps() {
        return this->steps;
    }
};

class SimplePattern : public BasePattern {
    public:

    struct event {
        short note = NOTE_OFF;
        short velocity = DEFAULT_VELOCITY;
        short channel = 0;
    };

    event *events = nullptr;
    BaseOutput *output = nullptr;

    SimplePattern() : BasePattern() {
        this->events = (event*)calloc(sizeof(event), steps);
    }

    virtual void set_output(BaseOutput *output) {
        this->output = output;
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

    virtual void process_step(int step) {
        //Serial.printf("process_step(%i)\t");
        if (this->query_note_off_for_step((step-1) % this->steps) && this->note_held) {
            //Serial.printf("%i: note off for step!");
            this->trigger_off_for_step(step);
        }
        if (this->query_note_on_for_step(step)) {
            //Serial.printf("query_note_on_for_step!,");
            this->trigger_on_for_step(step);
        }
        //Serial.println();
    };

    virtual void trigger_on_for_step(int step);
    virtual void trigger_off_for_step(int step);

};


#endif