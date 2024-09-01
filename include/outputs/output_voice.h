#pragma once

#include "outputs/output_processor.h"
#include "chord_player.h"

#include "bpm.h"

#include "parameter_inputs/VoltageParameterInput.h"

#include "menu.h"

class CVChordVoice;
extern CVChordVoice *cv_chord_output_1;
extern CVChordVoice *cv_chord_output_2;
extern CVChordVoice *cv_chord_output_3;

void setup_cv_pitch_inputs ();
void setup_cv_pitch_inputs_menu ();

class CVChordVoice : public BaseOutputProcessor {
    BaseParameterInput *pitch_input = nullptr;
    BaseParameterInput *velocity_input = nullptr;
    int8_t channel = 1;
    uint16_t colour;

    char label[MENU_C_MAX];

    MIDIOutputWrapper *output_wrapper = nullptr;

    ChordPlayer chord_player = ChordPlayer(
        [=](int8_t channel, int8_t note, int8_t velocity) -> void { output_wrapper->sendNoteOn(note, velocity, channel ); },
        [=](int8_t channel, int8_t note, int8_t velocity) -> void { output_wrapper->sendNoteOff(note, velocity, channel ); }
    );

    public:
        CVChordVoice(const char *label, MIDIOutputWrapper *output_wrapper, BaseParameterInput *initial_pitch_input, BaseParameterInput *initial_velocity_input, int8_t channel) {
            this->output_wrapper = output_wrapper;
            this->pitch_input = initial_pitch_input;
            this->velocity_input = initial_velocity_input;
            this->channel = channel;
            this->colour = menu->get_next_colour();
            strncpy(this->label, label, MENU_C_MAX);
        }

        virtual void process() override {
            int8_t new_note = NOTE_OFF;
            if (this->pitch_input!=nullptr && this->pitch_input->supports_pitch()) {
                VoltageParameterInput *voltage_source_input = (VoltageParameterInput*)this->pitch_input;
                new_note = voltage_source_input->get_voltage_pitch();
            }
            int velocity = MIDI_MAX_VELOCITY;
            if (this->velocity_input!=nullptr) {
                velocity = constrain(((float)MIDI_MAX_VELOCITY)*(float)this->velocity_input->get_normal_value_unipolar(), 0, MIDI_MAX_VELOCITY);
                //if (this->debug) Serial.printf("setting velocity to %i (%2.2f)\n", velocity, this->velocity_input->get_normal_value_unipolar());
            }
            this->chord_player.on_pre_clock(ticks, new_note, velocity);        
        }

        virtual void create_menu_items() {
            menu->add_page(this->label, this->colour);
            menu->add(chord_player.make_menu_items(), this->colour);
        }
};