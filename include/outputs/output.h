#include <Arduino.h>
#include <LinkedList.h>

// class to receive triggers from a sequencer and do something with them
class BaseOutput {
    public:
    
    virtual void receive_event(byte event_value_1, byte event_value_2, byte event_value_3) = 0;
    virtual void reset() = 0;
};

#include <Adafruit_TinyUSB.h>
#include "MIDI.h"
#include "Drums.h"
#include "bpm.h"

#include "sequencer/Sequencer.h"

class MIDIDrumOutput : public BaseOutput {
    public:

    byte note_number = -1;
    byte event_value_1, event_value_2, event_value_3;

    MIDIDrumOutput(byte note_number) {
        this->note_number = note_number;
    }

    virtual void receive_event(byte event_value_1, byte event_value_2, byte event_value_3) override {
        this->event_value_1 += event_value_1;
        this->event_value_2 += event_value_2;
        this->event_value_3 = event_value_3;
    }

    virtual bool should_go_on() {
        if (this->event_value_1==1)
            return true;
        return false;
    }
    virtual bool should_go_off() {
        if (this->event_value_2==1)
            return true;
        return false;
    }
    virtual void went_on() {
        this->event_value_1 -= 1;
    }
    virtual void went_off() {
        this->event_value_2 -= 1;
    }

    virtual void reset() {
        this->event_value_1 = this->event_value_2 = this->event_value_3 = 0;
    }
};


class MIDIOutputProcessor {
    public:
    LinkedList<MIDIDrumOutput*> nodes = LinkedList<MIDIDrumOutput*>();

    midi::MidiInterface<midi::SerialMIDI<Adafruit_USBD_MIDI>> *midi = nullptr;

    MIDIOutputProcessor(midi::MidiInterface<midi::SerialMIDI<Adafruit_USBD_MIDI>> *midi) {
        this->midi = midi;

        this->nodes.add(new MIDIDrumOutput(GM_NOTE_ACOUSTIC_BASS_DRUM));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_ACOUSTIC_SNARE));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_OPEN_HI_HAT));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_PEDAL_HI_HAT));
        this->nodes.add(new MIDIDrumOutput(GM_NOTE_CLOSED_HI_HAT));
    }

    //virtual void on_tick(uint32_t ticks) {
        //if (is_bpm_on_sixteenth(ticks)) {
    virtual void process() {
        for (int i = 0 ; i < this->nodes.size() ; i++) {
            if (this->nodes.get(i)->should_go_off()) {
                Serial.printf("Sending note off for node %i on note_number %i\n", i, this->nodes.get(i)->note_number);
                midi->sendNoteOff(this->nodes.get(i)->note_number, 0, 10);
                this->nodes.get(i)->went_off();
            }
            if (this->nodes.get(i)->should_go_on()) {
                Serial.printf("Sending note on for node %i on note_number %i\n", i, this->nodes.get(i)->note_number);
                midi->sendNoteOn(this->nodes.get(i)->note_number, 127, 10);
                this->nodes.get(i)->went_on();
            }
        }
    }

    virtual void configure_sequencer(BaseSequencer *sequencer) {
        for (int i = 0 ; i < this->nodes.size() ; i++) {
            sequencer->configure_pattern_output(i, this->nodes.get(i));
        }
    }
};