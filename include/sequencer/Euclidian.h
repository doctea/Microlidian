#ifndef EUCLIDIAN__INCLUDED
#define EUCLIDIAN__INCLUDED

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

class EuclidianPattern : public SimplePattern {
    public:
    
    struct arguments_t {
        int steps = 16;
        int pulses = steps/2;
        int rotation = 1;
        int duration = 1;
        float effective_euclidian_density = 0.75;
    };

    bool locked = false;

    bool initialised = false;

    bool active_status = true;
    //int pulses, rotation, duration;
    arguments_t arguments;
    arguments_t last_arguments;
    arguments_t default_arguments;
    int maximum_steps = steps;
    int tie_on = 0;

    float *global_density = nullptr;

    //EuclidianPattern() : SimplePattern() {}

    EuclidianPattern(int steps = 0, int pulses = 0, int rotation = -1, int duration = -1, int tie_on = -1) 
        //: arguments.pulses(pulses), arguments.rotation(arguments.rotation), arguments.duration(arguments.duration), tie_on(tie_on)
        : default_arguments { .steps = steps, .pulses = pulses, .rotation = rotation, .duration = duration }, tie_on(tie_on)
        {
            /*arguments.steps = steps;
            arguments.pulses = pulses;
            arguments.rotation = rotation;*/
            this->maximum_steps = steps > 0 ? steps : default_arguments.steps;
            if (steps>0)
                make_euclid(default_arguments.steps, default_arguments.pulses, default_arguments.rotation, default_arguments.duration, tie_on);
            //make_euclid();
        }

    virtual void restore_default_arguments() override {
        this->set_arguments(&this->default_arguments);
    }
    virtual void store_current_arguments_as_default() override {
        set_default_arguments(&this->arguments);
    }

    virtual void set_default_arguments(arguments_t *default_arguments_to_use) {
        memcpy(&this->default_arguments, default_arguments_to_use, sizeof(arguments_t));
    }
    virtual void set_arguments(arguments_t *arguments_to_use) {
        memcpy(&this->arguments, arguments_to_use, sizeof(arguments_t));
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

    void make_euclid(int steps = 0, int pulses = 0, int rotation = -1, int duration = -1, int trigger = -1, int tie_on = -1) { //}, float effective_euclidian_density = 0.75f) {
        if (steps > 0)      this->arguments.steps = steps;
        if (pulses > 0)     this->arguments.pulses = pulses;
        if (rotation >= 0)  this->arguments.rotation = rotation;
        if (duration >= 0)  this->arguments.duration = duration;
        if (tie_on >= 0)    this->tie_on = tie_on;

        this->arguments.effective_euclidian_density = *this->global_density;

        if (initialised && 0==memcmp(&this->arguments, &this->last_arguments, sizeof(arguments_t))) {
            // nothing changed, dont do anything
            return;
        }

        int original_pulses = this->arguments.pulses;
        //int temp_pulses = arguments.pulses * (1.5f*(MINIMUM_DENSITY+effective_euclidian_density));
        int temp_pulses = arguments.pulses * (1.5f*(MINIMUM_DENSITY+*global_density));
        //this->arguments.pulses * (1.5f*(MINIMUM_DENSITY+effective_euclidian_density));

        if (arguments.steps > maximum_steps) {
            //messages_log_add(String("arguments.steps (") + String(arguments.steps) + String(") is more than maximum steps (") + String(maximum_steps) + String(")"));
            maximum_steps = arguments.steps;
        }

        int bucket = 0;
        for (int i = 0 ; i < this->arguments.steps ; i++) {
            bucket += temp_pulses;
            if (bucket >= this->arguments.steps) {
                bucket -= this->arguments.steps;
                this->set_event_for_tick(i * ticks_per_step);
            } else {
                this->unset_event_for_tick(i * ticks_per_step);
            }
        }
        //this->maximum_steps = this->arguments.steps;
        
        if (this->arguments.rotation > 0) {
            this->rotate_pattern(this->arguments.rotation);
        }

        if (!initialised) {
            this->set_default_arguments(&this->arguments);
            initialised = true;
        }

        memcpy(&this->last_arguments, &this->arguments, sizeof(arguments_t));
    }

    // rotate the pattern around specifed number of steps -- TODO: could actually not change the pattern and just use the rotation in addition to offset in the query_patterns
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
        //Serial.println("todo: mutate the pattern");
        int r = random(0, 100);
        if (r > 50) {
            if (r > 75) {
                this->arguments.pulses += 1;
            } else {
                this->arguments.pulses -= 1;
            }
        } else if (r < 25) {
            this->arguments.rotation += 1;
        } else if (r > 25) {
            this->arguments.pulses *= 2;
        } else {
            this->arguments.pulses /= 2;
        }
        if (this->arguments.pulses >= this->arguments.steps || this->arguments.pulses <= 0) {
            this->arguments.pulses = 1;
        }
        //r = random(this->steps/2, this->maximum_steps);
        //this->arguments.steps = r;
        this->make_euclid();
    }

    virtual byte get_steps() override {
        return arguments.steps;
    }
    virtual void set_steps(byte steps) override {
        arguments.steps = steps;
    }
    virtual byte get_pulses() {
        return arguments.pulses;
    }
    virtual void set_pulses(byte pulses) {
        arguments.pulses = pulses;
    }
    virtual byte get_rotation() {
        return arguments.rotation;
    }
    virtual void set_rotation(byte rotation) {
        arguments.rotation = rotation;
    }
    virtual byte get_duration() {
        return arguments.duration;
    }
    virtual void set_duration(byte duration) {
        arguments.duration = duration;
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
        void create_menu_items(Menu *menu, int index);
    #endif
};

class EuclidianSequencer : public BaseSequencer {
    // todo: list of EuclidianPatterns...
    EuclidianPattern **patterns = nullptr;

    int seed = 0;
    int mutate_minimum_pattern = 0, mutate_maximum_pattern = number_patterns;
    bool reset_before_mutate = true, mutate_enabled = true, fills_enabled = true, add_phrase_to_seed = true;

    float global_density = 0.75f;

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
        return this->patterns[pattern];
    }

    void initialise_patterns() {
        #define SEQUENCE_LENGTH_STEPS 16
        const int LEN = SEQUENCE_LENGTH_STEPS;
        int i = 0;
        this->patterns[i++]->make_euclid(LEN,    4, 1,   DEFAULT_DURATION); //, TRIGGER_KICK);// get_trigger_for_pitch(GM_NOTE_ELECTRIC_BASS_DRUM));    // kick
        this->patterns[i++]->make_euclid(LEN,    5, 1,   DEFAULT_DURATION); //, TRIGGER_SIDESTICK); //get_trigger_for_pitch(GM_NOTE_SIDE_STICK));    // stick
        this->patterns[i++]->make_euclid(LEN,    2, 5,   DEFAULT_DURATION); //, TRIGGER_CLAP); //get_trigger_for_pitch(GM_NOTE_HAND_CLAP));    // clap
        this->patterns[i++]->make_euclid(LEN,    3, 1,   DEFAULT_DURATION); //, TRIGGER_SNARE); //get_trigger_for_pitch(GM_NOTE_ELECTRIC_SNARE));   // snare
        this->patterns[i++]->make_euclid(LEN,    3, 3,   DEFAULT_DURATION); //, TRIGGER_CRASH_1); //get_trigger_for_pitch(GM_NOTE_CRASH_CYMBAL_1));    // crash 1
        this->patterns[i++]->make_euclid(LEN,    7, 1,   DEFAULT_DURATION); //, TRIGGER_TAMB); //get_trigger_for_pitch(GM_NOTE_TAMBOURINE));    // tamb
        this->patterns[i++]->make_euclid(LEN,    9, 1,   DEFAULT_DURATION); //, TRIGGER_HITOM); //get_trigger_for_pitch(GM_NOTE_HIGH_TOM));    // hi tom!
        this->patterns[i++]->make_euclid(LEN/4,  2, 3,   DEFAULT_DURATION); //, TRIGGER_LOTOM); //get_trigger_for_pitch(GM_NOTE_LOW_TOM));    // low tom
        this->patterns[i++]->make_euclid(LEN/2,  2, 3,   DEFAULT_DURATION); //, TRIGGER_PEDALHAT); //get_trigger_for_pitch(GM_NOTE_PEDAL_HI_HAT));    // pedal hat
        this->patterns[i++]->make_euclid(LEN,    4, 3,   DEFAULT_DURATION); //, TRIGGER_OPENHAT); //get_trigger_for_pitch(GM_NOTE_OPEN_HI_HAT));    // open hat
        this->patterns[i++]->make_euclid(LEN,    16, 0,  0               ); //, TRIGGER_CLOSEDHAT); //get_trigger_for_pitch(GM_NOTE_CLOSED_HI_HAT)); //DEFAULT_DURATION);   // closed hat
        this->patterns[i++]->make_euclid(LEN*2,  1, 1,   DEFAULT_DURATION); //, TRIGGER_CRASH_2); //get_trigger_for_pitch(GM_NOTE_CRASH_CYMBAL_2));   // crash 2
        this->patterns[i++]->make_euclid(LEN*2,  1, 5,   DEFAULT_DURATION); //, TRIGGER_SPLASH); //get_trigger_for_pitch(GM_NOTE_SPLASH_CYMBAL));   // splash
        this->patterns[i++]->make_euclid(LEN*2,  1, 9,   DEFAULT_DURATION); //, TRIGGER_VIBRA); //get_trigger_for_pitch(GM_NOTE_VIBRA_SLAP));    // vibra
        this->patterns[i++]->make_euclid(LEN*2,  1, 13,  DEFAULT_DURATION); //, TRIGGER_RIDE_BELL); //get_trigger_for_pitch(GM_NOTE_RIDE_BELL));   // bell
        this->patterns[i++]->make_euclid(LEN*2,  5, 13,  DEFAULT_DURATION); //, TRIGGER_RIDE_CYM); //get_trigger_for_pitch(GM_NOTE_RIDE_CYMBAL_1));   // cymbal
        this->patterns[i++]->make_euclid(LEN,    4, 3,   2); //, PATTERN_BASS, 6);  // bass (neutron) offbeat with 6ie of 6
        this->patterns[i++]->make_euclid(LEN,    4, 3,   1); //, PATTERN_MELODY); //NUM_TRIGGERS+NUM_ENVELOPES);  // melody as above
        this->patterns[i++]->make_euclid(LEN,    4, 1,   4); //, PATTERN_PAD_ROOT); // root pad
        this->patterns[i++]->make_euclid(LEN,    4, 5,   4); //,   PATTERN_PAD_PITCH); // root pad
    }
    void reset_patterns() {
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
            int tick_of_step = tick % (PPQN/STEPS_PER_BEAT);
            if (tick_of_step==(PPQN/STEPS_PER_BEAT)-1) {
                for (int i = 0 ; i < number_patterns ; i++) {
                    if (!patterns[i]->is_locked()) {
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
            this->on_step(tick / (PPQN/STEPS_PER_BEAT));
        }
    };
    virtual void on_step(int step) override {
        for (int i = 0 ; i < number_patterns ; i++) {
            this->patterns[i]->process_step(step);
        }
    };
    virtual void on_beat(int beat) override {

    };
    virtual void on_bar (int bar) override {
        if (fills_enabled && bar == BARS_PER_PHRASE - 1) {
            // do fill
            for (int i = 0 ; i < 3 ; i++) {
                int ran = random(0/*euclidian_mutate_minimum_pattern % NUM_PATTERNS*/, constrain(1 + mutate_maximum_pattern, 0, number_patterns));
                if (!patterns[ran]->is_locked()) {
                    this->patterns[ran]->arguments.rotation += 2;
                    this->patterns[ran]->make_euclid();
                }
            }
            for (int i = 0 ; i < 3 ; i++) {
                int ran = random(mutate_minimum_pattern % number_patterns, constrain(1 + mutate_maximum_pattern, 0, number_patterns));
                if (!patterns[ran]->is_locked()) {
                    this->patterns[ran]->arguments.pulses *= 2;
                    if (this->patterns[ran]->arguments.pulses > this->patterns[ran]->arguments.steps) 
                        this->patterns[ran]->arguments.pulses /= 8;
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
              for (int i = 0 ; i < 3 ; i++) {
                // choose a pattern to mutate, out of all those for whom mutate is enabled
                int ran = random(mutate_minimum_pattern % number_patterns, constrain(1 + mutate_maximum_pattern, 0, number_patterns));
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

};

#endif