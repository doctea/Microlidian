#include <Arduino.h>
#include <LinkedList.h>

#include "debug.h"

#include <Adafruit_TinyUSB.h>
#include "MIDI.h"
#include "Drums.h"
#include "bpm.h"

#include "sequencer/Sequencer.h"

#include "midi_helpers.h"

// class to receive triggers from a sequencer and return values to the owner Processor
class BaseOutput {
    public:
    
    // event_value_1 = send a note on
    // event_value_2 = send a note off
    // event_value_3 = ??
    virtual void receive_event(byte event_value_1, byte event_value_2, byte event_value_3) = 0;
    virtual void reset() = 0;
};

// an output that tracks MIDI drum triggers
class MIDIDrumOutput : public BaseOutput {
    public:

    byte note_number = -1, last_note_number = -1;
    byte channel = 10;
    byte event_value_1, event_value_2, event_value_3;

    MIDIDrumOutput(byte note_number) {
        this->note_number = note_number;
    }

    virtual byte get_note_number() {
        return this->note_number;
    }
    virtual byte get_last_note_number() {
        return this->last_note_number;
    }
    virtual void set_last_note_number(byte note_number) {
        this->last_note_number = note_number;
    }
    virtual byte get_channel() {
        return this->channel;
    }

    virtual void receive_event(byte event_value_1, byte event_value_2, byte event_value_3) override {
        this->event_value_1 += event_value_1;
        this->event_value_2 += event_value_2;
        this->event_value_3 += event_value_3;
    }

    virtual bool should_go_on() {
        if (this->event_value_1>0)
            return true;
        return false;
    }
    virtual bool should_go_off() {
        if (this->event_value_2>0)
            return true;
        return false;
    }
    virtual void went_on() {
        this->event_value_1 -= 1;
    }
    virtual void went_off() {
        this->event_value_2 -= 1;
    }

    // forget the last message
    virtual void reset() {
        this->event_value_1 = this->event_value_2 = this->event_value_3 = 0;
    }
};

// class that counts up all active triggers from passed-in nodes, and calculates a note from that
class MIDINoteTriggerCountOutput : public MIDIDrumOutput {
    public:
        byte octave = 3;
        LinkedList<MIDIDrumOutput*> *nodes = nullptr;
        int base_note = SCALE_ROOT_A * octave;

        MIDINoteTriggerCountOutput(LinkedList<MIDIDrumOutput*> *nodes) : MIDIDrumOutput(0) {
            this->channel = 1;
            this->nodes = nodes;
        }

        virtual byte get_note_number() override {
            int count = 0;
            for (int i = 0 ; i < this->nodes->size() ; i++) {
                MIDIDrumOutput *o = this->nodes->get(i);
                if (o==this) continue;
                count += o->should_go_on() ? (i%12) : 0;
            }
            Debug_printf("get_note_number in MIDINoteTriggerCountOutput is %i\n", count);
            //return base_note + quantise_pitch(count);
            return quantise_pitch(count, SCALE_ROOT_C, 0);
        }
};

// holds individual output nodes and processes them (eg queries them for the pitch and sends note on/offs)
class BaseOutputProcessor {
    public:
        virtual void process() = 0;
};

// handles MIDI output; 
// possibly todo: move MIDIOutputWrapper stuff out of usb_midi_clocker and into a library and use that here?
class MIDIOutputProcessor : public BaseOutputProcessor {
    public:

    LinkedList<MIDIDrumOutput*> nodes = LinkedList<MIDIDrumOutput*>();
    midi::MidiInterface<midi::SerialMIDI<Adafruit_USBD_MIDI>> *midi = nullptr;

    MIDIOutputProcessor(midi::MidiInterface<midi::SerialMIDI<Adafruit_USBD_MIDI>> *midi) : BaseOutputProcessor() {
        this->midi = midi;

        /*this->nodes.add(new MIDIDrumOutput(GM_NOTE_ELECTRIC_BASS_DRUM));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_ELECTRIC_SNARE));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_OPEN_HI_HAT));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_PEDAL_HI_HAT));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_CLOSED_HI_HAT));*/
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_ELECTRIC_BASS_DRUM));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_SIDE_STICK));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_HAND_CLAP));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_ELECTRIC_SNARE));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_CRASH_CYMBAL_1));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_TAMBOURINE));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_HIGH_TOM));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_LOW_TOM));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_PEDAL_HI_HAT));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_OPEN_HI_HAT));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_CLOSED_HI_HAT));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_CRASH_CYMBAL_2));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_SPLASH_CYMBAL));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_VIBRA_SLAP));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_RIDE_BELL));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_RIDE_CYMBAL_1));
        this->nodes.add(new MIDINoteTriggerCountOutput(&this->nodes));
    }

    //virtual void on_tick(uint32_t ticks) {
        //if (is_bpm_on_sixteenth(ticks)) {

    // ask all the nodes to do their thing; send the results out to our output device
    virtual void process() {
        Debug_println("process-->");
        static int count = 0;
        //midi->sendNoteOff(35 + count, 0, 1);
        for (int i = 0 ; i < this->nodes.size() ; i++) {
            if(is_valid_note(this->nodes.get(i)->last_note_number)) {
                midi->sendNoteOff(this->nodes.get(i)->last_note_number, 0, this->nodes.get(i)->get_channel());
                this->nodes.get(i)->set_last_note_number(NOTE_OFF);
            }
        }
        count = 0;
        for (int i = 0 ; i < this->nodes.size() ; i++) {
            MIDIDrumOutput *o = this->nodes.get(i);
            Debug_printf("\tnode %i\n", i);
            if (o->should_go_off()) {
                int note_number = o->get_last_note_number();
                Debug_printf("\t\tgoes off note %i (%s), ", note_number, get_note_name_c(note_number));
                //Serial.printf("Sending note off for node %i on note_number %i chan %i\n", i, o->get_note_number(), o->get_channel());
                midi->sendNoteOff(note_number, 0, o->get_channel());
                //this->nodes.get(i)->went_off();
            }
            if (o->should_go_on()) {
                int note_number = o->get_note_number();
                Debug_printf("\t\tgoes on note %i (%s), ", note_number, get_note_name_c(note_number));
                //Serial.printf("Sending note on  for node %i on note_number %i chan %i\n", i, o->get_note_number(), o->get_channel());
                o->set_last_note_number(note_number);
                midi->sendNoteOn(note_number, MIDI_MAX_VELOCITY, o->get_channel());
                //this->nodes.get(i)->went_on();
                //count += i;
            }
            Debug_println();
        }
        /*if (count>0) {
            Serial.printf("sending combo note %i\n", count);
            midi->sendNoteOn(35 + count, 127, 1);
            //count = 35;
        }*/

        for (int i = 0 ; i < this->nodes.size() ; i++) {
            this->nodes.get(i)->reset();
        }

        Debug_println(".end.");
    }

    // configure target sequencer to use the output nodes held by this OutputProcessor
    virtual void configure_sequencer(BaseSequencer *sequencer) {
        for (int i = 0 ; i < this->nodes.size() ; i++) {
            sequencer->configure_pattern_output(i, this->nodes.get(i));
        }
        //sequencer->configure_pattern_output(0, this->nodes.get(0));
    }
};