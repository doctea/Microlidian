#pragma once

#include "outputs/output_processor.h"
#include "cv_pitch_trigger_core.h"
#include "cv_pitch_input_wiring.h"

#include "bpm.h"

#include "parameter_inputs/VoltageParameterInput.h"

#include "menu.h"

#include "midi_helpers.h"

#include "mymenu/menuitems_outputselectorcontrol.h"

#include "outputs/SeqlibSaveableSettings.h"

class CVPitchTrigger;
extern CVPitchTrigger *cv_pitch_input_1;
extern CVPitchTrigger *cv_pitch_input_2;
extern CVPitchTrigger *cv_pitch_input_3;

void setup_cv_pitch_inputs(BaseOutputProcessor *output_processor);
void setup_cv_pitch_inputs_menu();

class CVPitchTrigger
#ifdef ENABLE_STORAGE
    : virtual public SHDynamic<1, 5>  // 1 child; voice settings
#endif
{
    uint16_t colour;

    bool enabled = false;

    char label[MENU_C_MAX];

    BaseOutput *output_target = nullptr;
    LinkedList<BaseOutput *> *available_outputs = nullptr;

    CVPitchTriggerCore trigger_core = CVPitchTriggerCore(
        [=](int8_t note, int8_t velocity) -> void {
            if (!is_valid_note(note))
                return;
            if (!this->enabled)
                return;
            if (this->output_target == nullptr)
                return;
            output_target->receive_event(1, 0, note, velocity); // event_value_1 = send a note on; event_value_2 = send a note off; event_value_3 = note value (0-127); event_value_4 = velocity value (0-127)
        },
        [=](int8_t note, int8_t velocity) -> void {
            if (!is_valid_note(note))
                return;
            if (this->output_target == nullptr)
                return;
            output_target->receive_event(0, 1, note, velocity);
        },
        [=](int8_t note, int8_t velocity) -> void {
            if (!is_valid_note(note))
                return;
            if (this->output_target == nullptr)
                return;
            output_target->receive_event(0, 1, note, velocity);
        }
    );

public:
    CVPitchTrigger(
        const char *label,
        BaseOutput *output_target,
        LinkedList<BaseOutput *> *available_outputs,
        BaseParameterInput *initial_pitch_input,
        BaseParameterInput *initial_velocity_input
    ) {
        this->output_target = output_target;
        this->available_outputs = available_outputs;
        this->trigger_core.set_parameter_input_pitch(initial_pitch_input);
        this->trigger_core.set_parameter_input_velocity(initial_velocity_input);
        this->colour = menu->get_next_colour();
        strncpy(this->label, label, MENU_C_MAX);
        this->label[MENU_C_MAX - 1] = '\0';
#ifdef ENABLE_STORAGE
        this->set_path_segment(label);
#endif
    }

    virtual void process() {
        this->trigger_core.on_pre_clock(ticks);
    }

    virtual void set_enabled(bool v) {
        if (this->enabled && !v) {
            this->trigger_core.stop_all();
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
        this->trigger_core.set_parameter_input_pitch(parameter_input);
    }
    BaseParameterInput *get_parameter_input_pitch() {
        return this->trigger_core.get_parameter_input_pitch();
    }
    void set_parameter_input_velocity(BaseParameterInput *parameter_input) {
        this->trigger_core.set_parameter_input_velocity(parameter_input);
    }
    BaseParameterInput *get_parameter_input_velocity() {
        return this->trigger_core.get_parameter_input_velocity();
    }

    virtual void create_menu_items() {
        menu->add_page(this->label, this->colour);
        SubMenuItemBar *selectors = new SubMenuItemBar("Inputs");
        selectors->add(new LambdaToggleControl(
            "Enabled",
            [=](bool value) { this->enabled = value; },
            [=]() -> bool { return this->enabled; }
        ));

        CVPitchInputWiring::add_parameter_input_selectors<CVPitchTrigger>(
            selectors,
            this,
            &CVPitchTrigger::set_parameter_input_pitch,
            &CVPitchTrigger::get_parameter_input_pitch,
            &CVPitchTrigger::set_parameter_input_velocity,
            &CVPitchTrigger::get_parameter_input_velocity,
            "Pitch",
            "Velocity"
        );

        OutputSelectorControl<CVPitchTrigger> *output_selector = new OutputSelectorControl<CVPitchTrigger>(
            "Output",
            this,
            &CVPitchTrigger::set_output_target,
            &CVPitchTrigger::get_output_target,
            this->available_outputs,
            this->output_target
        );
        selectors->add(output_selector);
        menu->add(selectors, this->colour);
        menu->add(this->trigger_core.get_pitch_trigger()->make_menu_items(), this->colour);
    }

    virtual void set_output_by_name(const char *output_name) {
        if (this->available_outputs != nullptr) {
            for (size_t i = 0; i < this->available_outputs->size(); i++) {
                BaseOutput *o = this->available_outputs->get(i);
                if (o->matches_label(output_name)) {
                    this->set_output_target(o);
                    return;
                }
            }
        }
    }

    virtual const char *get_output_label() {
        if (this->output_target != nullptr)
            return this->output_target->label;
        return "None";
    }

#ifdef ENABLE_STORAGE
    virtual void setup_saveable_settings() override {
        SHDynamic<1, 5>::setup_saveable_settings();

        register_child(this->trigger_core.get_pitch_trigger());

        register_setting(
            CVPitchInputWiring::make_group_and_name_parameter_input_setting<CVPitchTrigger>(
                "Pitch Parameter Input",
                "ModInputs",
                this,
                &CVPitchTrigger::set_parameter_input_pitch,
                &CVPitchTrigger::get_parameter_input_pitch,
                ""
            ),
            SL_SCOPE_SCENE | SL_SCOPE_PROJECT
        );

        register_setting(
            CVPitchInputWiring::make_group_and_name_parameter_input_setting<CVPitchTrigger>(
                "Velocity Parameter Input",
                "ModInputs",
                this,
                &CVPitchTrigger::set_parameter_input_velocity,
                &CVPitchTrigger::get_parameter_input_velocity,
                ""
            ),
            SL_SCOPE_SCENE | SL_SCOPE_PROJECT
        );

        register_setting(new LOutputSaveableSetting(
                             "MIDI Output Target",
                             "ChordVoice",
                             [=](const char *output_label) -> void { this->set_output_by_name(output_label); },
                             [=](void) -> const char * { return this->get_output_label(); }
                         ),
                         SL_SCOPE_SCENE | SL_SCOPE_PROJECT);

        register_setting(new LSaveableSetting<bool>(
                             "Enabled",
                             "ChordVoice",
                             &this->enabled,
                             [=](bool value) { this->set_enabled(value); },
                             [=]() -> bool { return this->enabled; }
                         ),
                         SL_SCOPE_SCENE | SL_SCOPE_PROJECT);
    }
#endif
};
