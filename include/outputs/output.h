#ifndef OUTPUT__INCLUDED
#define OUTPUT__INCLUDED

#include <Arduino.h>
#include <LinkedList.h>

#include "debug.h"

#include <Adafruit_TinyUSB.h>
#include "MIDI.h"
#include "Drums.h"
#include "bpm.h"

#include "sequencer/Sequencer.h"

#include "midi_helpers.h"

#define MAX_LABEL 20

// class to receive triggers from a sequencer and return values to the owner Processor
class BaseOutput {
    public:
    char label[MAX_LABEL];
    BaseOutput (const char *label) {
        strncpy(this->label, label, MAX_LABEL);
    }
    
    // event_value_1 = send a note on
    // event_value_2 = send a note off
    // event_value_3 = ??
    virtual void receive_event(byte event_value_1, byte event_value_2, byte event_value_3) = 0;
    virtual void reset() = 0;
    virtual bool matches_label(const char *compare) {
        return strcmp(compare, this->label)==0;
    }

    virtual bool should_go_on() = 0;
    virtual bool should_go_off() = 0;

    virtual void stop() {};
    virtual void process() {};
};

// an output that tracks MIDI drum triggers
class MIDIDrumOutput : public BaseOutput {
    public:

    byte note_number = -1, last_note_number = -1;
    byte channel = 10;
    byte event_value_1, event_value_2, event_value_3;

    midi::MidiInterface<midi::SerialMIDI<Adafruit_USBD_MIDI>> *midi = nullptr;

    MIDIDrumOutput(const char *label, byte note_number, midi::MidiInterface<midi::SerialMIDI<Adafruit_USBD_MIDI>> *midi) : BaseOutput(label) {
        this->note_number = note_number;
        this->midi = midi;
    }

    virtual void stop() override {
        if(is_valid_note(last_note_number)) {
            midi->sendNoteOff(last_note_number, 0, this->get_channel());
            this->set_last_note_number(NOTE_OFF);
        }
    }
    virtual void process() override {
        if (should_go_off()) {
            int note_number = get_last_note_number();
            Debug_printf("\t\tgoes off note %i (%s), ", note_number, get_note_name_c(note_number));
            //Serial.printf("Sending note off for node %i on note_number %i chan %i\n", i, o->get_note_number(), o->get_channel());
            midi->sendNoteOff(note_number, 0, get_channel());
            //this->nodes.get(i)->went_off();
        }
        if (should_go_on()) {
            int note_number = get_note_number();
            Debug_printf("\t\tgoes on note %i (%s), ", note_number, get_note_name_c(note_number));
            //Serial.printf("Sending note on  for node %i on note_number %i chan %i\n", i, o->get_note_number(), o->get_channel());
            set_last_note_number(note_number);
            midi->sendNoteOn(note_number, MIDI_MAX_VELOCITY, get_channel());
            //this->nodes.get(i)->went_on();
            //count += i;
        }
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
        LinkedList<BaseOutput*> *nodes = nullptr;
        int base_note = SCALE_ROOT_A * octave;

        MIDINoteTriggerCountOutput(const char *name, LinkedList<BaseOutput*> *nodes, midi::MidiInterface<midi::SerialMIDI<Adafruit_USBD_MIDI>> *midi) 
            : MIDIDrumOutput(name, 0, midi) {
            this->channel = 1;
            this->nodes = nodes;
        }

        virtual byte get_note_number() override {
            int count = 0;
            for (int i = 0 ; i < this->nodes->size() ; i++) {
                BaseOutput *o = this->nodes->get(i);
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

    LinkedList<BaseOutput*> nodes = LinkedList<BaseOutput*>();
    midi::MidiInterface<midi::SerialMIDI<Adafruit_USBD_MIDI>> *midi = nullptr;

    MIDIOutputProcessor(midi::MidiInterface<midi::SerialMIDI<Adafruit_USBD_MIDI>> *midi) : BaseOutputProcessor() {
        this->midi = midi;

        /*this->nodes.add(new MIDIDrumOutput(GM_NOTE_ELECTRIC_BASS_DRUM));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_ELECTRIC_SNARE));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_OPEN_HI_HAT));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_PEDAL_HI_HAT));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_CLOSED_HI_HAT));*/
        this->nodes.add(new MIDIDrumOutput("Kick",          GM_NOTE_ELECTRIC_BASS_DRUM, midi));
        this->nodes.add(new MIDIDrumOutput("Stick",         GM_NOTE_SIDE_STICK, midi));
        this->nodes.add(new MIDIDrumOutput("Clap",          GM_NOTE_HAND_CLAP, midi));
        this->nodes.add(new MIDIDrumOutput("Snare",         GM_NOTE_ELECTRIC_SNARE, midi));
        this->nodes.add(new MIDIDrumOutput("Cymbal 1",      GM_NOTE_CRASH_CYMBAL_1, midi));
        this->nodes.add(new MIDIDrumOutput("Tamb",          GM_NOTE_TAMBOURINE, midi));
        this->nodes.add(new MIDIDrumOutput("HiTom",         GM_NOTE_HIGH_TOM, midi));
        this->nodes.add(new MIDIDrumOutput("LoTom",         GM_NOTE_LOW_TOM, midi));
        this->nodes.add(new MIDIDrumOutput("PHH",           GM_NOTE_PEDAL_HI_HAT, midi));
        this->nodes.add(new MIDIDrumOutput("OHH",           GM_NOTE_OPEN_HI_HAT, midi));
        this->nodes.add(new MIDIDrumOutput("CHH",           GM_NOTE_CLOSED_HI_HAT, midi));
        this->nodes.add(new MIDIDrumOutput("Cymbal 2",      GM_NOTE_CRASH_CYMBAL_2, midi));
        this->nodes.add(new MIDIDrumOutput("Splash",        GM_NOTE_SPLASH_CYMBAL, midi));
        this->nodes.add(new MIDIDrumOutput("Vibra",         GM_NOTE_VIBRA_SLAP, midi));
        this->nodes.add(new MIDIDrumOutput("Ride Bell",     GM_NOTE_RIDE_BELL, midi));
        this->nodes.add(new MIDIDrumOutput("Ride Cymbal",   GM_NOTE_RIDE_CYMBAL_1, midi));
        this->nodes.add(new MIDINoteTriggerCountOutput("Bass", &this->nodes, midi));
    }

    //virtual void on_tick(uint32_t ticks) {
        //if (is_bpm_on_sixteenth(ticks)) {

    // ask all the nodes to do their thing; send the results out to our output device
    virtual void process() {
        Debug_println("process-->");
        static int count = 0;
        //midi->sendNoteOff(35 + count, 0, 1);
        for (int i = 0 ; i < this->nodes.size() ; i++) {
            BaseOutput *n = this->nodes.get(i);
            n->stop();
        }
        count = 0;
        for (int i = 0 ; i < this->nodes.size() ; i++) {
            BaseOutput *o = this->nodes.get(i);
            Debug_printf("\tnode %i\n", i);
            o->process();
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

#endif