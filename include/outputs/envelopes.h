#pragma once

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

class EnvelopeOutput : public MIDIDrumOutput/*, public EnvelopeBase*/ {
    public:

    byte midi_cc = -1;

    EnvelopeBase *envelope;

    /*#ifdef ENABLE_SCREEN
        using EnvelopeBase::make_menu_items;
    #endif*/

    EnvelopeOutput(const char *label, byte note_number, byte cc_number, byte channel, MIDIOutputWrapper *output_wrapper) : 
        MIDIDrumOutput(label, note_number, channel, output_wrapper)
        //,EnvelopeBase(label),
        ,midi_cc(cc_number)
        {
            // todo: allow to switch to different types of envelope..?
            this->envelope = new RegularEnvelope(label);
        }

    virtual void process() override {
        bool x_should_go_off = should_go_off();
        bool x_should_go_on  = should_go_on();

        if (x_should_go_off) {
            this->envelope->update_state(0, false, ticks);
        }
        if (x_should_go_on) {
            this->envelope->update_state(127, true, ticks);
        }

        //this->process_envelope(millis());

        //MIDIDrumOutput::process();    // ?
    }

    virtual void loop() override {
        this->envelope->process_envelope(ticks);
    }

    // todo: make EnvelopeBase accept a lambda as callback, instead of the send_envelope_level override
    virtual void send_envelope_level(uint8_t level) override {
        output_wrapper->sendControlChange(midi_cc, level, channel);
    }

    #ifdef ENABLE_SCREEN
        virtual void make_menu_items(Menu *menu, int index) override {
            //EnvelopeBase::make_menu_items(menu, index);
            this->envelope->make_menu_items(menu, index);
        };
    #endif
};


