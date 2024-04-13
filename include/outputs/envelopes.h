#pragma once

#include "Config.h"
#ifndef DEFAULT_ENVELOPE_CLASS
    #define DEFAULT_ENVELOPE_CLASS Weirdolope
    //#define DEFAULT_ENVELOPE_CLASS RegularEnvelope
#endif

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
#endif

#include "envelopes/envelopes.h"
#include "envelopes/borolope.h"

class EnvelopeOutput : public MIDIDrumOutput {
    public:

    byte midi_cc = -1;

    EnvelopeBase *envelope;

    EnvelopeOutput(const char *label, byte note_number, byte cc_number, byte channel, MIDIOutputWrapper *output_wrapper) : 
        MIDIDrumOutput(label, note_number, channel, output_wrapper)
        ,midi_cc(cc_number)
        {
            // todo: allow to switch to different types of envelope..?
            this->envelope = new DEFAULT_ENVELOPE_CLASS(
                label, 
                [=](uint8_t level) -> void { output_wrapper->sendControlChange(this->midi_cc, level, this->channel); } 
            );
        }

    virtual void process() override {
        if (!is_enabled()) return;

        bool x_should_go_off = should_go_off();
        bool x_should_go_on  = should_go_on();

        if (x_should_go_off) {
            this->envelope->update_state(0, false, ticks);
        }
        if (x_should_go_on) {
            this->envelope->update_state(127, true, ticks);
        }
    }

    virtual void loop() override {
        this->envelope->process_envelope(ticks);
    }

    #ifdef ENABLE_SCREEN
        virtual void make_menu_items(Menu *menu, int index) override {
            this->envelope->make_menu_items(menu, index);
        };
    #endif

    #ifdef ENABLE_CV_INPUT
        virtual LinkedList<FloatParameter*> *get_parameters() override {
            return this->envelope->get_parameters();
        }
    #endif
};


