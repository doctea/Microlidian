#ifndef SEQUENCER__INCLUDED
#define SEQUENCER__INCLUDED

class SimplePattern;
class BaseOutput;
class FloatParameter;
class Menu;

class BaseSequencer {
    public:

    bool running = true;
    int number_patterns = 20;
    virtual SimplePattern *get_pattern(int pattern) = 0;

    /*virtual void process_tick(int tick) {
        static int last_processed = -1;
        if (tick == last_processed) return;

        if (!running) return;

        //this->process_tick_internal(tick);
        */

        /*if (is_tick_on_phrase(tick))
            this->on_phrase(phrase_number);
        if (is_tick_on_bar(tick))
            this->on_bar(tick);
        if (is_tick_on_beat(tick))
            this->on_beat(tick);*/
        
    //}
    virtual bool is_running() {
        return this->running;
    }
    virtual void set_playing(bool state = true) {
        this->running = state;
    }

    //virtual void process_tick_internal(int tick) = 0;

    virtual void on_loop(int tick) = 0;
    virtual void on_tick(int tick) = 0;
    virtual void on_step(int step) = 0;
    virtual void on_step_end(int step) = 0;
    virtual void on_beat(int beat) = 0;
    virtual void on_bar(int bar) = 0;
    virtual void on_phrase(int phrase) = 0;

    virtual void configure_pattern_output(int index, BaseOutput *output);
    
    #if defined(ENABLE_CV_INPUT)
        virtual LinkedList<FloatParameter*> *getParameters();
    #endif

    #if defined(ENABLE_SCREEN)
        virtual void make_menu_items(Menu *menu);
    #endif

};

#endif