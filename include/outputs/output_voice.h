#pragma once

#include "outputs/output_processor.h"
#include "chord_player.h"

#include "bpm.h"

#include "parameter_inputs/VoltageParameterInput.h"

#include "menu.h"

#include "midi_helpers.h"

#include "mymenu_items/ParameterInputMenuItems.h"

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

    IMIDINoteTarget *output_target = nullptr;

    ChordPlayer chord_player = ChordPlayer(
        [=](int8_t channel, int8_t note, int8_t velocity) -> void { output_target->sendNoteOn(note, velocity, channel ); },
        [=](int8_t channel, int8_t note, int8_t velocity) -> void { output_target->sendNoteOff(note, velocity, channel ); }
    );

    public:
        CVChordVoice(const char *label, IMIDINoteTarget *output_target, BaseParameterInput *initial_pitch_input, BaseParameterInput *initial_velocity_input, int8_t channel) {
            this->output_target = output_target;
            this->pitch_input = initial_pitch_input;
            this->velocity_input = initial_velocity_input;
            this->channel = channel;
            this->colour = menu->get_next_colour();
            strncpy(this->label, label, MENU_C_MAX);
            
            this->set_path_segment(label);
        }

        virtual void process() override {
            int8_t new_note = NOTE_OFF;
            if (this->pitch_input!=nullptr && this->pitch_input->supports_pitch()) {
                new_note = pitch_input->get_voltage_pitch();
            }

            int velocity = MIDI_MAX_VELOCITY;
            if (this->velocity_input!=nullptr) {
                velocity = constrain(((float)MIDI_MAX_VELOCITY)*(float)this->velocity_input->get_normal_value_unipolar(), 0, MIDI_MAX_VELOCITY);
                //if (this->debug) Serial.printf("setting velocity to %i (%2.2f)\n", velocity, this->velocity_input->get_normal_value_unipolar());
            }

            this->chord_player.on_pre_clock(ticks, new_note, velocity);        
        }

        virtual void set_output_target(IMIDINoteTarget *output_target) {
            this->output_target = output_target;
        }

        void set_parameter_input_pitch(BaseParameterInput *parameter_input) {
            this->pitch_input = parameter_input;
        }
        BaseParameterInput *get_parameter_input_pitch() {
            return this->pitch_input;
        }
        void set_parameter_input_velocity(BaseParameterInput *parameter_input) {
            this->velocity_input = parameter_input;
        }
        BaseParameterInput *get_parameter_input_velocity() {
            return this->velocity_input;
        }

        virtual void create_menu_items() {
            menu->add_page(this->label, this->colour);
            SubMenuItemBar *selectors = new SubMenuItemBar("Inputs");
            selectors->add(new ParameterInputSelectorControl<CVChordVoice>(
                "Pitch", 
                this, 
                &CVChordVoice::set_parameter_input_pitch, 
                &CVChordVoice::get_parameter_input_pitch, 
                parameter_manager->get_available_pitch_inputs(), 
                this->pitch_input
            ));
            selectors->add(new ParameterInputSelectorControl<CVChordVoice>(
                "Velocity", 
                this, 
                &CVChordVoice::set_parameter_input_velocity, 
                &CVChordVoice::get_parameter_input_velocity, 
                parameter_manager->available_inputs, 
                this->velocity_input
            ));
            menu->add(selectors, this->colour);
            menu->add(chord_player.make_menu_items(), this->colour);
        }

        virtual void setup_saveable_settings() override {
            BaseOutputProcessor::setup_saveable_settings();

            register_setting(new LSaveableSetting<int8_t>(
                "MIDI channel",
                "ChordVoice",
                &this->channel,
                [=](int8_t v) { this->channel = v; },
                [=]() -> int8_t { return this->channel; }
            ), SL_SCOPE_SCENE | SL_SCOPE_PROJECT);
            
            register_child(&chord_player);

            // @@TODO: hmmm, do we need some way to be able to identify the target_output between sessions..?
            // but as its an IMIDINoteTarget pointer, we can't just save the pointer value... 
            // maybe we need some kind of unique ID for the output_target and then look it up on load?
            // actually it seems that its just a MIDIOutputWrapper, which will always point at our single
            // instance of MIDIOutputWrapper, so we can get away without this for now.
            //register_setting(new Output)

            // we do need to save the parameter_input_pitch and parameter_input_velocity though, so we will need some way to identify those on load as well; maybe we can just save the label of the parameter input and look it up on load?
            register_setting(new LSaveableSetting<const char*>(
                "Pitch Parameter Input",
                "ModInputs",
                nullptr,
                [=](const char* group_and_name) {
                    BaseParameterInput *input = parameter_manager->getInputForGroupAndName(group_and_name);
                    this->set_parameter_input_pitch(input);
                },
                [=]() -> const char* {
                    BaseParameterInput *input = this->get_parameter_input_pitch();
                    if (input==nullptr) return "";
                    return input->get_group_and_name();
                }
            ), SL_SCOPE_SCENE | SL_SCOPE_PROJECT);

            register_setting(new LSaveableSetting<const char*>(
                "Velocity Parameter Input",
                "ModInputs",
                nullptr,
                [=](const char* group_and_name) {
                    BaseParameterInput *input = parameter_manager->getInputForGroupAndName(group_and_name);
                    this->set_parameter_input_velocity(input);
                },
                [=]() -> const char* {
                    BaseParameterInput *input = this->get_parameter_input_velocity();
                    if (input==nullptr) return "";
                    return input->get_group_and_name();
                }
            ), SL_SCOPE_SCENE | SL_SCOPE_PROJECT);

        }
};