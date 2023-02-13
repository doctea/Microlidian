#include "Patterns.h"

#include "Sequencer.h"
#include <bpm.h>

#define MINIMUM_DENSITY 0.0f  // 0.10f
#define DEFAULT_DURATION 1

#define BARS_PER_PHRASE 4

class EuclidianPattern : public SimplePattern {
    public:

    bool active_status = true;
    int pulses, rotation, duration;
    int maximum_steps = steps;
    int tie_on = 0;

    //EuclidianPattern() : SimplePattern() {}

    EuclidianPattern(int steps = 0, int pulses = 0, int rotation = -1, int duration = -1, int tie_on = -1) 
        : pulses(pulses), rotation(rotation), duration(duration), tie_on(tie_on)
        {
            make_euclid(steps, pulses, rotation, duration, tie_on);
        }

    void make_euclid(int steps = 0, int pulses = 0, int rotation = -1, int duration = -1, int trigger = -1, int tie_on = -1, float effective_euclidian_density = 0.75f) {
        if (steps > 0) this->steps = steps;
        if (tie_on >=0) this->tie_on = tie_on;
        if (pulses > 0) this->pulses = pulses;
        if (rotation >= 0) this->rotation = rotation;
        if (duration >= 0) this->duration = duration;

        int original_pulses = this->pulses;
        this->pulses *= (1.5f*(MINIMUM_DENSITY+effective_euclidian_density));

        int bucket = 0;
        for (int i = 0 ; i < this->steps ; i++) {
            bucket += this->pulses;
            if (bucket >= this->steps) {
                bucket -= this->steps;
                this->set_event_for_tick(i * ticks_per_step);
            } else {
                this->unset_event_for_tick(i * ticks_per_step);
            }
        }
        this->maximum_steps = this->steps;
        
        if (this->rotation > 0) {
            this->rotate_pattern(this->rotation);
        }

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
        Serial.println("todo: mutate the pattern");
    }

    /*void trigger_on_for_step(int step) override {
        Serial.printf("trigger_on_for_step(%i)\n", step);
    }

    void trigger_off_for_step(int step) override {
        Serial.printf("trigger_off_for_step(%i)\n", step);
    }*/
};

class EuclidianSequencer : public BaseSequencer {
    // todo: list of EuclidianPatterns...
    EuclidianPattern **patterns = nullptr;

    int mutate_minimum_pattern = 0, mutate_maximum_pattern = number_patterns;
    bool reset_before_mutate, mutate_enabled, fills_enabled;

    int get_euclidian_seed() {
        return 0;
    }

    public:
    EuclidianSequencer() : BaseSequencer() {
        this->patterns = (EuclidianPattern**) calloc(number_patterns, sizeof(EuclidianPattern));
        for (int i = 0 ; i < number_patterns ; i++) {
            this->patterns[i] = new EuclidianPattern();
        }
    }

    SimplePattern *get_pattern(int pattern) {
        return this->patterns[pattern];
    }

    void reset_patterns() {
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

    virtual void on_loop(int tick) override {};
    virtual void on_tick(int tick) override {
        if (is_bpm_on_sixteenth(tick))
            //this->on_step(this->get_step_for_tick(tick));
            this->on_step(tick / (PPQN/4));
    };
    virtual void on_step(int step) override {
        for (int i = 0 ; i < number_patterns ; i++) {
            this->patterns[i]->process_step(step);
        }
    };
    virtual void on_beat(int beat) override {

    };
    virtual void on_bar (int bar) override {
        if (bar == BARS_PER_PHRASE -1) {
            // do fill
            for (int i = 0 ; i < 3 ; i++) {
                int ran = random(0/*euclidian_mutate_minimum_pattern % NUM_PATTERNS*/, constrain(1 + mutate_maximum_pattern, 0, number_patterns));
                this->patterns[ran]->rotation += 2;
                this->patterns[ran]->make_euclid();
            }
            for (int i = 0 ; i < 3 ; i++) {
                int ran = random(mutate_minimum_pattern % number_patterns, constrain(1 + mutate_maximum_pattern, 0, number_patterns));
                this->patterns[ran]->pulses *= 2;
                if (this->patterns[ran]->pulses > this->patterns[ran]->steps) 
                    this->patterns[ran]->pulses /= 8;
                this->patterns[ran]->make_euclid();
            }
        }
    };
    virtual void on_phrase(int phrase) override {
        if (reset_before_mutate && (mutate_enabled || fills_enabled)) {
            reset_patterns();

            if (mutate_enabled) {
              unsigned long seed = get_euclidian_seed();
              randomSeed(seed);
              for (int i = 0 ; i < 3 ; i++) {
                // choose a pattern to mutate, out of all those for whom mutate is enabled
                int ran = random(mutate_minimum_pattern % number_patterns, constrain(1 + mutate_maximum_pattern, 0, number_patterns));
                randomSeed(seed + ran);
                this->patterns[ran]->mutate();
              }
            }
        }
    };

};