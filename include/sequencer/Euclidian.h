#pragma once

#include "Config.h"

#include "debug.h"

#include <LinkedList.h>

#include "Patterns.h"

#include "Sequencer.h"
#include <bpm.h>

class FloatParameter;
class Menu;

#define MINIMUM_DENSITY 0.0f  // 0.10f
#define MAXIMUM_DENSITY 1.5f
#define DEFAULT_DURATION 1

#define BARS_PER_PHRASE 4

//float effective_euclidian_density = 0.75f;

#define SEQUENCE_LENGTH_STEPS 16
const int LEN = SEQUENCE_LENGTH_STEPS;
    
struct arguments_t {
    int_fast8_t steps = SEQUENCE_LENGTH_STEPS;
    int_fast8_t pulses = steps/2;
    int_fast8_t rotation = 1;
    int_fast8_t duration = 1;
    float effective_euclidian_density = 0.6666;
    int_fast8_t tie_on = -1;
};

extern arguments_t initial_arguments[];

class EuclidianPattern : public SimplePattern {
    public:

    bool locked = false;

    bool initialised = false;

    bool active_status = true;
    //int pulses, rotation, duration;
    arguments_t arguments;
    arguments_t last_arguments;
    arguments_t default_arguments;
    arguments_t used_arguments;
    int maximum_steps = steps;
    //int tie_on = 0;

    float *global_density = nullptr;

    //EuclidianPattern() : SimplePattern() {}

    EuclidianPattern(int steps = MAX_STEPS, int pulses = 0, int rotation = -1, int duration = -1, int tie_on = -1) 
        //: arguments.pulses(pulses), arguments.rotation(arguments.rotation), arguments.duration(arguments.duration), tie_on(tie_on)
        : SimplePattern(), default_arguments { .steps = steps, .pulses = pulses, .rotation = rotation, .duration = duration, .tie_on = tie_on }
        {
            /*arguments.steps = steps;
            arguments.pulses = pulses;
            arguments.rotation = rotation;*/
            this->maximum_steps = steps > 0 ? steps : default_arguments.steps;
            //if (steps>0)
                //make_euclid(default_arguments.steps, default_arguments.pulses, default_arguments.rotation, default_arguments.duration, tie_on);
            //make_euclid();
            set_arguments(&default_arguments);
            make_euclid();
        }

    virtual void restore_default_arguments() override {
        this->set_arguments(&this->default_arguments);
    }
    virtual void store_current_arguments_as_default() override {
        set_default_arguments(&this->arguments);
    }

    virtual void set_default_arguments(arguments_t *default_arguments_to_use) {
        memcpy(&this->default_arguments, default_arguments_to_use, sizeof(arguments_t));
        set_arguments(&this->default_arguments);
    }
    virtual void set_arguments(arguments_t *arguments_to_use) {
        memcpy(&this->arguments, arguments_to_use, sizeof(arguments_t));
        memcpy(&this->used_arguments, arguments_to_use, sizeof(arguments_t));
    }

    virtual const char *get_summary() override {
        static char summary[32];
        snprintf(summary, 32, 
            "%-2i %-2i %-2i", // [%c]",
            last_arguments.steps, last_arguments.pulses, last_arguments.rotation
            //this->query_note_on_for_step(BPM_CURRENT_STEP_OF_BAR) ? 'X' : ' '
        );
        return summary;
    }

    /*void make_euclid(arguments_t arguments) {
        this->make_euclid(
            arguments.steps,
            arguments.pulses,
            arguments.rotation,
            arguments.duration,
            arguments.tie_on
        );
    }*/

    //void make_euclid(int steps = 0, int pulses = 0, int rotation = -1, int duration = -1, /*, int trigger = -1,*/ int tie_on = -1) { //}, float effective_euclidian_density = 0.75f) {
    void make_euclid() {
        this->used_arguments.effective_euclidian_density = *this->global_density;
        //if (Serial) Serial.println("make_euclid..");

        if (initialised && 0==memcmp(&this->used_arguments, &this->last_arguments, sizeof(arguments_t))) {
            //if (Serial) Serial.println("nothing changed, don't do anything");
            // nothing changed, dont do anything
            return;
        }

        /*if (!initialised) {
            this->set_default_arguments(&this->used_arguments);
            initialised = true;
        */

        int original_pulses = this->used_arguments.pulses;
        float multiplier = 1.5f*(MINIMUM_DENSITY+*global_density);
        int temp_pulses = 0.5f + (((float)original_pulses) * multiplier);
        //if (Serial) {
            //Serial.printf("from original pulses %i, got temp_pulses %i from multiplier %3.3f (global_density=%3.3f)\n", original_pulses, temp_pulses, multiplier, *global_density);
            //Serial_flush();
        //}
        //int temp_pulses = original_pulses;    // for disabling density for testing purposes

        if (used_arguments.steps > maximum_steps) {
            //messages_log_add(String("arguments.steps (") + String(arguments.steps) + String(") is more than maximum steps (") + String(maximum_steps) + String(")"));
            maximum_steps = used_arguments.steps;
        }

        int bucket = 0;
        //if (this->used_arguments.rotation!=0)
        for (int i = 0 ; i < this->used_arguments.steps ; i++) {
            int rotation = this->used_arguments.rotation;
            //int new_i = ((used_arguments.steps - rotation) + i) % used_arguments.steps;
            int new_i = (rotation + i) % used_arguments.steps;
            bucket += temp_pulses;
            if (bucket >= this->used_arguments.steps) {
                bucket -= this->used_arguments.steps;
                this->set_event_for_tick(new_i * ticks_per_step);
            } else {
                this->unset_event_for_tick(new_i * ticks_per_step);
            }
        }
        //this->maximum_steps = this->arguments.steps;
        
        /*if (this->used_arguments.rotation > 0) {
            this->rotate_pattern(this->used_arguments.rotation);
        }*/

        memcpy(&this->last_arguments, &this->used_arguments, sizeof(arguments_t));
    }

    // rotate the pattern around specifed number of steps -- 
    // TODO: could actually not change the pattern and just use the rotation in addition to offset in the query_patterns
    void rotate_pattern(int rotate) {
        unsigned long rotate_time = millis();
        bool temp_pattern[steps];
        int offset = steps - rotate;
        for (int i = 0 ; i < steps ; i++) {
            //stored[i] = p->stored[abs( (i + offset) % p->steps )];
            temp_pattern[i] = this->query_note_on_for_tick(
                (abs( (i + offset) % steps) * ticks_per_step)
            );
        }
        for (int i = 0 ; i < steps ; i++) {
            if (temp_pattern[i])
                this->set_event_for_tick(i * ticks_per_step);
            else 
                this->unset_event_for_tick(i * ticks_per_step);
        }
    }  

    void mutate() {
        //if (Serial) Serial.println("mutate the pattern!");
        int r = random(0, 100);
        if (r > 50) {
            if (r > 75) {
                this->set_pulses(this->get_pulses() + 1);
            } else {
                this->set_pulses(this->get_pulses() - 1);
            }
        } else if (r < 25) {
            this->set_rotation(this->get_rotation() + 1);
        } else if (r > 25) {
            this->set_pulses(this->get_pulses() * 2);
        } else {
            this->set_pulses(this->get_pulses() / 2);
        }
        if (this->get_pulses() >= this->get_steps() || this->get_pulses() <= 0) {
            this->set_pulses(1);
        }
        //r = random(this->steps/2, this->maximum_steps);
        //this->arguments.steps = r;
        this->make_euclid();
    }

    virtual byte get_effective_steps() override {
        return this->used_arguments.steps;
    }

    virtual byte get_steps() override {
        return this->arguments.steps;
    }
    virtual void set_steps(byte steps) override {
        this->arguments.steps = steps;
        this->used_arguments.steps = steps;
    }
    virtual byte get_pulses() {
        return this->arguments.pulses;
    }
    virtual void set_pulses(byte pulses) {
        this->arguments.pulses = pulses;
        this->used_arguments.pulses = pulses;
    }
    virtual byte get_rotation() {
        return this->arguments.rotation;
    }
    virtual void set_rotation(byte rotation) {
        this->arguments.rotation = rotation;
        this->used_arguments.rotation = rotation;
    }
    virtual byte get_duration() {
        return this->arguments.duration;
    }
    virtual void set_duration(byte duration) {
        this->arguments.duration = duration;
        used_arguments.duration = duration;
    }

    virtual int get_tick_duration() {
        return this->arguments.duration;
    }

    virtual bool is_locked() {
        return this->locked;
    }
    virtual void set_locked(bool state) {
        this->locked = state;
    }

    /*void trigger_on_for_step(int step) override {
        Serial.printf("trigger_on_for_step(%i)\n", step);
    }

    void trigger_off_for_step(int step) override {
        Serial.printf("trigger_off_for_step(%i)\n", step);
    }*/

    #ifdef ENABLE_SCREEN
        virtual void create_menu_items(Menu *menu, int index) override;
    #endif
    
    #if defined(ENABLE_CV_INPUT)
        virtual LinkedList<FloatParameter*> *getParameters(int i) override;
    #endif
};



class EuclidianSequencer : public BaseSequencer {
    // todo: list of EuclidianPatterns...
    EuclidianPattern **patterns = nullptr;

    int seed = 0;
    uint_fast8_t mutate_minimum_pattern = 0, mutate_maximum_pattern = number_patterns;
    int_fast8_t mutation_count = 3, effective_mutation_count = 3;
    bool    reset_before_mutate = true, 
            mutate_enabled = true, 
            fills_enabled = true, 
            add_phrase_to_seed = true;
    float global_density = 0.6666f;

    public:
    EuclidianSequencer() : BaseSequencer() {
        EuclidianPattern *p = nullptr;
        this->patterns = (EuclidianPattern**) calloc(number_patterns, sizeof(p));
        for (int i = 0 ; i < number_patterns ; i++) {
            this->patterns[i] = new EuclidianPattern();
            this->patterns[i]->global_density = &this->global_density;
        }
    }

    float get_density() {
        return this->global_density;
    }
    void set_density(float v) {
        this->global_density = v;
    }

    bool is_mutate_enabled() {
        return this->mutate_enabled;
    }
    void set_mutated_enabled(bool v = true) {
        this->mutate_enabled = v;
    }

    int_fast8_t get_effective_mutation_count() {
        return effective_mutation_count;
    }
    void set_mutation_count(int_fast8_t v) {
        this->mutation_count = v;
    }

    bool should_reset_before_mutate() {
        return this->reset_before_mutate;
    }
    void set_reset_before_mutate(bool v = true) {
        this->reset_before_mutate = v;
    }

    bool is_fills_enabled() {
        return this->fills_enabled;
    }
    void set_fills_enabled(bool v = true) {
        this->fills_enabled = v;
    }

    bool is_add_phrase_enabled() {
        return this->add_phrase_to_seed;
    }
    void set_add_phrase_enabled(bool v = true) {
        this->add_phrase_to_seed = v;
    }
    
    int get_euclidian_seed() {
        return seed + add_phrase_to_seed ? BPM_CURRENT_PHRASE : 0; //BPM_CURRENT_PHRASE;
    }
    void set_euclidian_seed(int seed) {
        this->seed = seed;
    }
    
    SimplePattern *get_pattern(int pattern) {
        if (pattern < 0 || pattern >= number_patterns)
            return nullptr;
        return this->patterns[pattern];
    }

    void initialise_patterns() {
        for (int i = 0 ; i < number_patterns ; i++) {
            this->patterns[i]->set_default_arguments(&initial_arguments[i]);
        }
    }
    void reset_patterns() {
        //if (Serial) Serial.println("reset_patterns!");
        for (int i = 0 ; i < number_patterns ; i++) {
            EuclidianPattern *p = (EuclidianPattern*)this->get_pattern(i);
            if (!p->is_locked()) {
                p->restore_default_arguments();
                p->make_euclid();
            }
        }
    }

    virtual void on_loop(int tick) override {};
    virtual void on_tick(int tick) override {
        #ifdef MUTATE_EVERY_TICK
            int_fast8_t tick_of_step = tick % TICKS_PER_STEP;
            if (tick_of_step==TICKS_PER_STEP-1) {
                for (int_fast8_t i = 0 ; i < number_patterns ; i++) {
                    if (!patterns[i]->is_locked()) {
                        //if (Serial) Serial.println("mutate every tick!");
                        this->patterns[i]->make_euclid();
                    }
                }
            }
        #endif
        if (is_bpm_on_phrase(tick)) {
            this->on_phrase(BPM_CURRENT_PHRASE);
        }
        if (is_bpm_on_bar(tick)) {
            this->on_bar(BPM_CURRENT_BAR_OF_PHRASE);
        }
        if (is_bpm_on_beat(tick)) {
            this->on_beat(BPM_CURRENT_BEAT_OF_BAR);
        }
        if (is_bpm_on_sixteenth(tick)) {
            //this->on_step(this->get_step_for_tick(tick));
            this->on_step(tick / TICKS_PER_STEP);
        } /*else if (is_bpm_on_sixteenth(tick,1)) {
            this->on_step_end(tick / (PPQN/STEPS_PER_BEAT));
        }*/
        for (int_fast8_t i = 0 ; i < number_patterns ; i++) {
            this->patterns[i]->process_tick(tick);
        }

    };
    virtual void on_step(int step) override {
        for (int_fast8_t i = 0 ; i < number_patterns ; i++) {
            this->patterns[i]->process_step(step);
        }
    };
    virtual void on_step_end(int step) override {
        for (int_fast8_t i = 0 ; i < number_patterns ; i++) {
            this->patterns[i]->process_step_end(step);
        }
    }
    virtual void on_beat(int beat) override {

    };
    virtual void on_bar(int bar) override {
        //if (Serial) Serial.println("on_bar!");
        if (fills_enabled && bar == BARS_PER_PHRASE - 1) {
            //if (Serial) Serial.println("on_bar doing fill!");
            // do fill
            for (int_fast8_t i = 0 ; i < 3 ; i++) {
                int_fast8_t ran = random(mutate_minimum_pattern % number_patterns, constrain(1 + mutate_maximum_pattern, 0, number_patterns));
                if (!patterns[ran]->is_locked()) {
                    //this->patterns[ran]->arguments.rotation += 2;
                    this->patterns[ran]->set_rotation(this->patterns[ran]->get_rotation() + 2);
                    this->patterns[ran]->make_euclid();
                }
            }
            for (int_fast8_t i = 0 ; i < 3 ; i++) {
                int_fast8_t ran = random(mutate_minimum_pattern % number_patterns, constrain(1 + mutate_maximum_pattern, 0, number_patterns));
                if (!patterns[ran]->is_locked()) {
                    this->patterns[ran]->set_rotation(this->patterns[ran]->get_rotation() * 2);
                    //this->patterns[ran]->arguments.pulses *= 2;
                    if (this->patterns[ran]->get_pulses() > this->patterns[ran]->get_steps()) 
                        this->patterns[ran]->set_pulses(this->patterns[ran]->get_pulses() / 8);
                    this->patterns[ran]->make_euclid();
                }
            }
        }
    };
    virtual void on_phrase(int phrase) override {
        if (reset_before_mutate)
            reset_patterns();
        if (mutate_enabled) {
            if (mutate_enabled) {
              unsigned long seed = get_euclidian_seed();
              randomSeed(seed);
              for (uint_fast8_t i = 0 ; i < this->mutation_count ; i++) {
                // choose a pattern to mutate, out of all those for whom mutate is enabled
                uint_fast8_t ran = random(mutate_minimum_pattern % number_patterns, constrain(1 + mutate_maximum_pattern, 0, number_patterns));
                randomSeed(seed + ran);
                if (!patterns[ran]->is_locked()) {
                    this->patterns[ran]->mutate();
                }
              }
            }
        }
    };
    
    #if defined(ENABLE_CV_INPUT)
        virtual LinkedList<FloatParameter*> *getParameters() override;
    #endif

    #if defined(ENABLE_SCREEN)
        virtual void make_menu_items(Menu *menu) override;
    #endif

};

