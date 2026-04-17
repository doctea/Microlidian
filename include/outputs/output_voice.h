#pragma once

#include "outputs/output_processor.h"
#include "chord_player.h"

#include "bpm.h"

#include "parameter_inputs/VoltageParameterInput.h"

#include "menu.h"

#include "midi_helpers.h"

#include "mymenu_items/ParameterInputMenuItems.h"
#include "mymenu/menuitems_outputselectorcontrol.h"

#include "outputs/SeqlibSaveableSettings.h"

class CVChordVoice;
extern CVChordVoice *cv_chord_output_1;
extern CVChordVoice *cv_chord_output_2;
extern CVChordVoice *cv_chord_output_3;

void setup_cv_pitch_inputs(BaseOutputProcessor *output_processor);
void setup_cv_pitch_inputs_menu();

class CVChordVoice /*: public BaseOutputProcessor*/ 
    #ifdef ENABLE_STORAGE
        : virtual public SHDynamic<1, 3>  // no children; settings for this voice
    #endif
    {
    BaseParameterInput *pitch_input = nullptr;
    BaseParameterInput *velocity_input = nullptr;
    uint16_t colour;

    bool enabled = false;
    
    char label[MENU_C_MAX];
    
    BaseOutput *output_target = nullptr;
    LinkedList<BaseOutput*> *available_outputs = nullptr;

    ChordPlayer chord_player = ChordPlayer(
        [=](int8_t channel, int8_t note, int8_t velocity) -> void { 
            //output_target->sendNoteOn(note, velocity, channel ); 
            if (!is_valid_note(note)) 
                return;
            if (!this->enabled) 
                return;
            output_target->receive_event(1, 0, note, velocity); // event_value_1 = send a note on; event_value_2 = send a note off; event_value_3 = note value (0-127); event_value_4 = velocity value (0-127)
        },
        [=](int8_t channel, int8_t note, int8_t velocity) -> void { 
            if (!is_valid_note(note)) 
                return;
            output_target->receive_event(0, 1, note, velocity); 
        }
    );

    public:
        CVChordVoice(
            const char *label, 
            BaseOutput *output_target, 
            LinkedList<BaseOutput*> *available_outputs, 
            BaseParameterInput *initial_pitch_input, 
            BaseParameterInput *initial_velocity_input
        ) {
            this->output_target = output_target;
            this->available_outputs = available_outputs;
            this->pitch_input = initial_pitch_input;
            this->velocity_input = initial_velocity_input;
            this->colour = menu->get_next_colour();
            strncpy(this->label, label, MENU_C_MAX);
            this->label[MENU_C_MAX - 1] = '\0';
            #ifdef ENABLE_STORAGE
                this->set_path_segment(label);
            #endif
        }

        virtual void process() {
            int8_t new_note = NOTE_OFF;
            if (this->pitch_input!=nullptr && this->pitch_input->supports_pitch()) {
                new_note = pitch_input->get_voltage_pitch();
            }

            int velocity = MIDI_MAX_VELOCITY;
            if (this->velocity_input!=nullptr) {
                velocity = constrain(
                    ((float)MIDI_MAX_VELOCITY) * (float)this->velocity_input->get_normal_value_unipolar(), 
                    0, 
                    MIDI_MAX_VELOCITY
                );
                //if (this->debug) Serial.printf("setting velocity to %i (%2.2f)\n", velocity, this->velocity_input->get_normal_value_unipolar());
            }

            this->chord_player.on_pre_clock(ticks, new_note, velocity);        
        }

        virtual void set_enabled(bool v) {
            if (this->enabled && !v) {
                this->chord_player.stop_all();
            }
            this->enabled = v;
        }

        virtual void set_output_target(BaseOutput *output_target) {
            this->output_target = output_target;
        }
        virtual BaseOutput *get_output_target() {
            return this->output_target;
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
            selectors->add(new LambdaToggleControl(
                "Enabled",
                [=](bool value) { this->enabled = value; },
                [=]() -> bool { return this->enabled; }
            ));
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
            OutputSelectorControl<CVChordVoice> *output_selector = new OutputSelectorControl<CVChordVoice> (
                "Output",
                this,
                &CVChordVoice::set_output_target,
                &CVChordVoice::get_output_target,
                this->available_outputs,
                this->output_target
            );
            selectors->add(output_selector);
            menu->add(selectors, this->colour);
            menu->add(chord_player.make_menu_items(), this->colour);
        }

        virtual void set_output_by_name(const char *output_name) {
            if (this->available_outputs!=nullptr) {
                for (size_t i = 0 ; i < this->available_outputs->size() ; i++) {
                    BaseOutput *o = this->available_outputs->get(i);
                    if (o->matches_label(output_name)) {
                        this->set_output_target(o);
                        return;
                    }
                }
            }
        }

        virtual const char *get_output_label() {        
            if (this->output_target!=nullptr)
                return this->output_target->label;
            return "None";
        }

        #ifdef ENABLE_STORAGE
        virtual void setup_saveable_settings() override {
            SHDynamic<1, 3>::setup_saveable_settings();

            // register_setting(new LSaveableSetting<int8_t>(
            //     "MIDI channel",
            //     "ChordVoice",
            //     &this->channel,
            //     [=](int8_t v) { this->channel = v; },
            //     [=]() -> int8_t { return this->channel; }
            // ), SL_SCOPE_SCENE | SL_SCOPE_PROJECT);
            
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

            register_setting(new LOutputSaveableSetting(
                "MIDI Output Target",
                "ChordVoice",
                [=](const char* output_label) -> void { this->set_output_by_name(output_label); },
                [=](void) -> const char* { return this->get_output_label(); }
            ), SL_SCOPE_SCENE | SL_SCOPE_PROJECT);

            register_setting(new LSaveableSetting<bool>(
                "Enabled",
                "ChordVoice",
                &this->enabled,
                [=](bool value) { this->set_enabled(value); },
                [=]() -> bool { return this->enabled; }
            ), SL_SCOPE_SCENE | SL_SCOPE_PROJECT);
        }
        #endif
};