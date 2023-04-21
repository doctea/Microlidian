#ifndef ENVELOPES_H__INCLUDED
#define ENVELOPES_H__INCLUDED

#include "bpm.h"
#include "output.h"

#include "SinTables.h"

#define SUSTAIN_MINIMUM   1   // was 32         // minimum sustain volume to use (below this becomes inaudible, so cut it off)
#define ENV_MAX_ATTACK    (PPQN*2) //48 // maximum attack stage length in ticks
#define ENV_MAX_HOLD      (PPQN*2) //48 // maximum hold stage length
#define ENV_MAX_DECAY     (PPQN*2) //48 // maximum decay stage length
#define ENV_MAX_RELEASE   (PPQN*4) //96 // maximum release stage length

#ifdef ENABLE_SCREEN
#include <LinkedList.h>
#include "mymenu.h"
#include "menuitems_object.h"
#endif

class EnvelopeOutput : public MIDIDrumOutput {
    public:

    EnvelopeOutput(const char *label, byte note_number, byte cc_number, MIDIOutputWrapper *output_wrapper) : 
        MIDIDrumOutput(label, note_number, output_wrapper), midi_cc(cc_number) {
        this->randomise();
    }

    enum stage : byte {
        OFF = 0,
        //DELAY,  // time
        ATTACK,
        HOLD, // time
        DECAY,
        SUSTAIN,
        RELEASE,
        //END = 0
        /*LFO_SYNC_RATIO_HOLD_AND_DECAY,
        LFO_SYNC_RATIO_SUSTAIN_AND_RELEASE,
        ASSIGN_HARMONY_OUTPUT*/
    };

    byte midi_cc = -1;

    //#ifndef TEST_LFOS
    byte stage = OFF;
    /*#else
    byte stage = LFO_SYNC_RATIO;
    #endif*/

    byte velocity = 127;         // triggered velocity
    byte actual_level = 0;          // right now, the level
    byte stage_start_level = 0;     // level at start of current stage

    // TODO: int delay_length = 5;                    // D - delay before atack starts
    unsigned int  attack_length   = 0;                // A - attack  - length of stage
    unsigned int  hold_length     = (PPQN / 4) - 1;   // H - hold    - length to hold at end of attack before decay
    unsigned int  decay_length    = (PPQN / 2) - 1;   // D - decay   - length of stage
    float         sustain_ratio   = 0.90f;            // S - sustain - level to drop to after decay phase
    unsigned int  release_length  = (PPQN / 2) - 1;   // R - release - length (time to drop to 0)

    byte lfo_sync_ratio_hold_and_decay = 0;
    byte lfo_sync_ratio_sustain_and_release = 0;

    unsigned long stage_triggered_at = 0;
    unsigned long triggered_at = 0; 
    unsigned long last_sent_at = 0;

    int trigger_on = 0; // 0->19 = trigger #, 20 = off, 32->51 = trigger #+loop, 64->84 = trigger #+invert, 96->116 = trigger #+loop+invert
    bool loop_mode = false;
    bool invert = false;

    byte last_sent_lvl; // value but not inverted
    byte last_sent_actual_lvl;  // actual midi value sent

    byte cc_value_sync_modifier = 1;

    void send_envelope_level(byte level) {
        output_wrapper->sendControlChange(midi_cc, level, 1);
    }

    void randomise() {
        //this->lfo_sync_ratio_hold_and_decay = (byte)random(0,127);
        //this->lfo_sync_ratio_sustain_and_release = (byte)random(0,127);
        this->set_attack((byte)random(0,64));
        this->set_hold((byte)random(0,127));
        this->set_decay((byte)random(0,127));
        this->set_sustain(random(64,127));
        this->set_release((byte)random(0,127));
        this->invert = (byte)random(0,10) < 2;
    }

    void kill() {
        this->stage = OFF;
        this->stage_start_level = (byte)0;
        send_envelope_level(0);
    }

    // received a message that the state of the envelope should change (note on/note off etc)
    void update_state (byte velocity, bool state) {
        unsigned long now = ticks; //clock_millis(); 
        unsigned long env_time = millis();
        if (state == true) { //&& this->stage==OFF) {  // envelope told to be in 'on' state by note on
            this->velocity = velocity;
            this->actual_level = velocity; // TODO: start this at 0 so it can ramp / offset level feature
            this->stage_start_level = velocity; // TODO: start this at 0 so it can ramp / offset level feature
            this->stage = ATTACK;
            this->triggered_at = now;
            this->stage_triggered_at = now;
            this->last_sent_at = 0;  // trigger immediate sending

            //NUMBER_DEBUG(7, this->stage, this->attack_length);
        } else if (state == false && this->stage != OFF) { // envelope told to be in 'off' state by note off
            // note ended - fastforward to next envelope stage...?
            // if attack/decay/sustain, go straight to release at the current volume...?
            switch (this->stage) {
            case RELEASE:
                // received note off while already releasing -- cut note short
                this->stage_start_level = 0; 
                this->stage = OFF;
                this->stage_triggered_at = now;
                return;
            case OFF:
                // don't do anything if we're in this stage and we receive note off, since we're already meant to be stopping by now
                /*NUMBER_DEBUG(15, 15, 15);
                NUMBER_DEBUG(15, 15, 15);
                NUMBER_DEBUG(15, 15, 15);
                NUMBER_DEBUG(15, 15, 15);*/
                return;

            // if in any other stage, jump straight to RELEASE stage at current volume
            case ATTACK:
            case HOLD:
                // TODO: continue to HOLD , but leap straight to RELEASE when reached end?
            case DECAY:
            case SUSTAIN:
            default:
                //NOISY_DEBUG(500, 2);
                //NUMBER_DEBUG(13, 13, 13);

                this->stage = RELEASE;
                this->stage_start_level = this->actual_level;
                this->stage_triggered_at = now;
                this->last_sent_at = 0;  // trigger immediate sending
                break;
            }
        }
    }

    virtual void process() override {
        bool x_should_go_off = should_go_off();
        bool x_should_go_on  = should_go_on();

        if (x_should_go_off) {
            this->update_state(0, false);
        }
        if (x_should_go_on) {
            this->update_state(127, true);
        }

        //this->process_envelope(millis());

        MIDIDrumOutput::process();
    }

    virtual void loop() override {
        this->process_envelope();
    }

    void process_envelope(unsigned long now = millis()) {
        now = ticks;
        unsigned long elapsed = now - this->stage_triggered_at;
        unsigned long real_elapsed = elapsed;    // elapsed is currently the number of REAL ticks that have passed

        byte lvl = this->stage_start_level;
        byte s = this->stage ;
        
        float ratio = (float)PPQN / (float)cc_value_sync_modifier;  // calculate ratio of real ticks : pseudoticks
        elapsed = (float)elapsed * ratio;   // convert real elapsed to pseudoelapsed
            
        #ifdef DEBUG_ENVELOPES
            static byte last_stage;
            if (s>0 && last_stage!=s && this->trigger_on_channel==TRIGGER_CHANNEL_LFO) {
                Serial.printf("process_envelope(%i, %u, trig %i) in stage %i: sync'd elapsed is %u, ", i, now, this->trigger_on_channel, elapsed);
                Serial.printf("real elapsed is %u, lvl is %i, ", real_elapsed, this->last_sent_lvl);
                Serial.printf("cc_value_sync_modifier is %u\r\n", cc_value_sync_modifier);

                /*Serial.printf("Ratio is PPQN %u / %u = %3.3f, so therefore", PPQN, cc_value_sync_modifier, ratio);
                Serial.printf("converting real elapsed %u to ", real_elapsed);
                Serial.printf("%u\r\n", elapsed);*/
            } 
            last_stage = s;
            
        #endif
        
        // TODO: switch() would be nicer than if-else blocks, but ran into weird problems (like breakpoints never being hit) when approached it that way?!
        /*if (s==LFO_SYNC_RATIO) {
        lvl = random(0,127); //(int) (127.0 * (0.5+isin( (this->lfo_sync_ratio/PPQN) * elapsed)));
        } else */
        if (s==ATTACK) {
            //NUMBER_DEBUG(8, this->stage, elapsed/16);
            // length of time to ramp up to level
            //MIDI.sendControlChange(7, this->level, 1);
            //NUMBER_DEBUG(8, this->stage, this->stage_start_level);

            if (this->attack_length==0) 
                lvl = this->velocity; // immediately start at desired velocity
            else
                lvl = (byte) ((float)this->velocity * ((float)elapsed / ((float)this->attack_length )));
            
            if (elapsed >= this->attack_length) {
                //NUMBER_DEBUG(9, this->stage, 1);
                this->stage++; // = HOLD;
                this->stage_triggered_at = now;
                this->stage_start_level = lvl;
            }
        } else if (s==HOLD) {
            lvl = this->velocity; //stage_start_level;
                if (elapsed >= this->hold_length || this->hold_length == 0) {
                this->stage++; // = DECAY;
                this->stage_triggered_at = now;
                this->stage_start_level = lvl;
            }
        } else if (s==DECAY) {
            //NUMBER_DEBUG(8, this->stage, this->stage_start_level);
            // length of time to decay down to sustain_level
            float f_sustain_level = SUSTAIN_MINIMUM + (this->sustain_ratio * (float)this->stage_start_level);
            float f_original_level = this->stage_start_level;

            if (this->decay_length>0) {
                float decay_position = ((float)elapsed / (float)(this->decay_length));
        
                // we start at stage_start_level
                float diff = (f_original_level - (f_sustain_level));
                // and over decay_length time
                // we want to scale down to f_sustain_level at minimum
        
                lvl = f_original_level - (diff * decay_position);
            } else {
                // if there's no decay stage then set level to the sustain level
                lvl = f_sustain_level; 
            }
            
            if (elapsed >= this->decay_length) {// || this->decay_length==0 || lvl < f_sustain_level) {
                //NUMBER_DEBUG(9, this->stage, 1);
                //if (this->decay_length==0) lvl = this->stage_start_level;

                this->stage++; // = SUSTAIN;
                this->stage_triggered_at = now;
                this->stage_start_level = lvl;
            }
        } else if (s==SUSTAIN) {
            //NUMBER_DEBUG(8, this->stage, elapsed/16); //this->stage_start_level);
            //float sustain_level = this->sustain_ratio * ((float)this->inital_level);
            // the volume level to remain at until the note is released
            byte sustain_level = this->stage_start_level;
            //sustain_level = random(0, 127);

            lvl = (byte)(sustain_level);
            
            // go straight to RELEASE if sustain is zero or we're in lfo mode
            if (this->sustain_ratio==0.0f || this->loop_mode) { //trigger_on>=TRIGGER_CHANNEL_LFO) {
                this->stage_triggered_at = now;
                this->stage_start_level = lvl;
                Debug_printf("Leaving SUSTAIN stage with lvl at %i\r\n", lvl);
                this->stage++; // = RELEASE;
            }
        } else if (s==RELEASE) {
            if (this->sustain_ratio==0.0f) {
                this->stage_start_level = this->velocity;
            }

            // the length of time to decay down to 0
            // immediately jump here if note off during any other stage (than OFF)
            if (this->release_length>0) {
                //float eR = (float)elapsed / (float)(0.1+this->release_length); 
                //float eR = (float)elapsed / (float)(this->release_length); 
                float eR = (float)elapsed / (float)(this->release_length); 
                eR = constrain(eR, 0.0f, 1.0f);
        
                //NUMBER_DEBUG(8, this->stage, this->stage_start_level);
                //Serial.printf("in RELEASE stage, release_length is %u, elapsed is %u, eR is %3.3f, lvl is %i ....", this->release_length, elapsed, eR, lvl);
                lvl = (byte)((float)this->stage_start_level * (1.0f-eR));
                //Serial.printf(".... lvl changed to %i\r\n", lvl);
            } else {
                lvl = 0;
            }
    
            if (elapsed > this->release_length || this->release_length==0) {
                //NUMBER_DEBUG(9, this->stage, 1);
                this->stage_triggered_at = now;
                this->stage = OFF;
                //Serial.printf("Leaving RELEASE stage with lvl at %i\r\n", lvl);
            } /*else {
            Serial.printf("RELEASE not finished because %u is less than %i?\r\n", elapsed, this->release_length);
            }*/
        } else if (s==OFF) {  // may have stopped or something so mute
            lvl = 0; //64;
        }

        // if lfo_sync_ratio is >=16 for this envelope then apply lfo modulation to the level
        // TODO: make this actually more useful... set upper/lower limits to modulation, elapsed-based scaling of modulation, only modulate during eg RELEASE stage
        if (this->stage!=OFF) {  // this is where we would enable them for constant LFO i think?
            // modulate the level
            // TODO: FIX THIS SO RATIO WORKS !
            //lvl = (lvl*(0.5+isin(elapsed * ((this->lfo_sync_ratio / 16) * PPQN)))); 
            //lvl = (lvl * (0.5 + isin(elapsed * (((this->lfo_sync_ratio) / 16 ))))); // * PPQN)))); ///((float)(cc_value_sync_modifier^2)/127.0))));  // TODO: find good parameters to use here, cc to adjust the modulation level etc
            
            int sync = (this->stage==DECAY || this->stage==HOLD) 
                            ?
                            this->lfo_sync_ratio_hold_and_decay
                            :
                        (this->stage==SUSTAIN || this->stage==RELEASE)
                            ?
                            this->lfo_sync_ratio_sustain_and_release : 
                        -1;
                            
            //NUMBER_DEBUG(12, 0, 127 * isin(elapsed
            if (sync>=0) {            
                sync *= 4; // multiply sync value so that it gives us possibility to modulate much faster

                float mod_amp = (float)lvl/4.0f; //32.0; // modulation amplitude is a quarter of the current level

                float mod_result = mod_amp * isin((float)elapsed * PPQN * ((float)sync/127.0f));
                //Serial.printf("mod_result is %3.1f, elapsed is %i, sync is %i\r\n", mod_result, elapsed, sync);
                
                lvl = constrain(
                    lvl + mod_result,
                    0,
                    127
                );
                //Serial.printf("sync of %i resulted in lvl %i\r\n", sync, lvl);
            } 
        } else {
            // envelope is stopped - restart it if in lfo mode!
            if (this->loop_mode) { //trigger_on>=TRIGGER_CHANNEL_LFO) {
                Debug_printf("envelope %i is stopped, restarting\n", i);
                update_state(this->velocity, true);
            }
        }

        /*if (this->trigger_on==TRIGGER_CHANNEL_LFO_MODULATED || this->trigger_on==TRIGGER_CHANNEL_LFO_MODULATED_AND_INVERTED ) {
        int modulating_envelope = (i-1==-1) ? NUM_ENVELOPES_EXTENDED-1 : i-1;
        lvl = ((float)lvl) * ((float)envelopes[modulating_envelope].last_sent_lvl/127);
        }*/

        this->actual_level = lvl;
        
        if (this->last_sent_actual_lvl != this->invert ? 127-lvl : lvl) {  // only send a new value if its different to the last value sent for this envelope
            //if (this->stage==OFF) lvl = 0;   // force level to 0 if the envelope is meant to be OFF

            /*if (this->invert) {
                lvl = 127-lvl;
            }*/
            send_envelope_level(this->invert ? 127-lvl : lvl); // send message to midimuso
            
            this->last_sent_at = now;
            this->last_sent_lvl = lvl;
            this->last_sent_actual_lvl = this->invert ? 127-lvl : lvl;
            /*if (this->invert)
                Serial.printf("sending value %i for envelope %i\n", this->last_sent_actual_lvl, i);*/

        } else {
            /*if (this->invert)
                Serial.printf("not sending for envelope %i cos already sent %i\n", i, this->last_sent_actual_lvl);*/
        }
    }

    int attack_value, hold_value, decay_value, sustain_value, release_value;

    virtual void set_attack(byte attack) {
        this->attack_value = attack;
        this->attack_length = (ENV_MAX_ATTACK) * ((float)attack/127.0f);
    }
    virtual byte get_attack() {
        return this->attack_value;
    }
    virtual void set_hold(byte hold) {
        this->hold_value = hold;
        this->hold_length = (ENV_MAX_HOLD) * ((float)hold/127.0f);
    }
    virtual byte get_hold() {
        return this->hold_value;
    }
    virtual void set_decay(byte decay) {
        this->decay_value = decay;
        decay_length   = (ENV_MAX_DECAY) * ((float)decay/127.0f);
    }
    virtual byte get_decay() {
        return this->decay_value;
    }
    virtual void set_sustain(byte sustain) {
        this->sustain_value = sustain; //(((float)value/127.0f) * (float)(128-SUSTAIN_MINIMUM)) / 127.0f;
        sustain_ratio = (((float)sustain/127.0f) * (float)(128-SUSTAIN_MINIMUM)) / 127.0f;
    }
    virtual byte get_sustain() {
        return this->sustain_value;
    }
    virtual void set_release(byte release) {
        this->release_value = release;
        release_length = (ENV_MAX_RELEASE) * ((float)release/127.0f);
    }
    virtual byte get_release() {
        return this->release_value;
    }
    virtual bool is_invert() {
        return invert;
    }
    virtual void set_invert(bool i) {
        this->invert = i;
    }

    #ifdef ENABLE_SCREEN
        virtual void make_menu_items(Menu *menu, int index) override {
            #ifdef ENABLE_ENVELOPE_MENUS
                char label[40];
                snprintf(label, 40, "Envelope %i", index);
                menu->add_page(label);

                menu->add(new ObjectNumberControl<EnvelopeOutput,byte>("Attack", this, &EnvelopeOutput::set_attack,     &EnvelopeOutput::get_attack,    nullptr, 0, 127, true, true));
                menu->add(new ObjectNumberControl<EnvelopeOutput,byte>("Hold", this, &EnvelopeOutput::set_hold,         &EnvelopeOutput::get_hold,      nullptr, 0, 127, true, true));
                menu->add(new ObjectNumberControl<EnvelopeOutput,byte>("Decay", this, &EnvelopeOutput::set_decay,       &EnvelopeOutput::get_decay,     nullptr, 0, 127, true, true));
                menu->add(new ObjectNumberControl<EnvelopeOutput,byte>("Sustain", this, &EnvelopeOutput::set_sustain,   &EnvelopeOutput::get_sustain,   nullptr, 0, 127, true, true));
                menu->add(new ObjectNumberControl<EnvelopeOutput,byte>("Release", this, &EnvelopeOutput::set_release,   &EnvelopeOutput::get_release,   nullptr, 0, 127, true, true));
                menu->add(new ObjectToggleControl<EnvelopeOutput>("Invert", this, &EnvelopeOutput::set_invert,          &EnvelopeOutput::is_invert,    nullptr));
            #endif
        }
    #endif


};

#endif