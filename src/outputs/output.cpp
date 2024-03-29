#include "Drums.h"
#include "outputs/output.h"
#include "outputs/output_processor.h"

#include "clock.h"

uint32_t external_cv_ticks_per_pulse_values[] = { 1, 2, 3, 4, 6, 8, 12, 16, 24 };
#ifdef ENABLE_CLOCK_INPUT_CV
    void set_external_cv_ticks_per_pulse_values(uint32_t new_value) {
        external_cv_ticks_per_pulse = new_value;
        //reset_clock();
        ticks = 0;
    }
    uint32_t get_external_cv_ticks_per_pulse_values() {
        return external_cv_ticks_per_pulse;
    }
#endif


// todo: different modes to correlate with the midimuso mode + output availability..
int8_t get_muso_note_for_drum(int8_t drum_note) {
    int8_t retval = 60;
    switch (drum_note) {
        case GM_NOTE_ACOUSTIC_BASS_DRUM:
        case GM_NOTE_ELECTRIC_BASS_DRUM:
            retval += TRIGGER_SIDESTICK; break;
        case GM_NOTE_HAND_CLAP:
            retval += TRIGGER_CLAP; break;
        case GM_NOTE_ACOUSTIC_SNARE:
        case GM_NOTE_ELECTRIC_SNARE:
            retval += TRIGGER_SNARE; break;
        case GM_NOTE_CRASH_CYMBAL_1:
            retval += TRIGGER_CRASH_1; break;
        case GM_NOTE_TAMBOURINE:
            retval += TRIGGER_TAMB; break;
        case GM_NOTE_LOW_TOM:
            retval += TRIGGER_LOTOM; break;
        case GM_NOTE_HIGH_TOM:
            retval += TRIGGER_HITOM; break;
        case GM_NOTE_PEDAL_HI_HAT:
            retval += TRIGGER_PEDALHAT; break;
        case GM_NOTE_OPEN_HI_HAT:
            retval += TRIGGER_OPENHAT; break;
        case GM_NOTE_CLOSED_HI_HAT:
            retval += TRIGGER_CLOSEDHAT; break;
    }     
    return retval;
}

note_message_t convert_note_for_muso_drum(int8_t pitch, int8_t velocity, int8_t channel) {
    return {
        get_muso_note_for_drum(pitch),
        velocity,
        channel==(int8_t)GM_CHANNEL_DRUMS ? (int8_t)MUSO_TRIGGER_CHANNEL : channel
    };
}

MIDIOutputWrapper *output_wrapper = nullptr;
MIDIOutputProcessor *output_processor = nullptr;

void setup_output() {
    output_wrapper = new MIDIOutputWrapper();
    output_processor = new MIDIOutputProcessor(output_wrapper);     // todo: set this up dynamically, probably reading from a config file
}

void setup_output_parameters() {
    output_wrapper->setup_parameters();
}

#ifdef ENABLE_SCREEN
    #include "mymenu.h"
    #include "menuitems_object_multitoggle.h"

    void setup_output_menu() {
        output_wrapper->create_menu_items();
        output_processor->create_menu_items();
    }

    void MIDIOutputProcessor::create_menu_items() {
        for (unsigned int i = 0 ; i < this->nodes->size() ; i++) {
            BaseOutput *node = this->nodes->get(i);
            node->make_menu_items(menu, i);
        }

        menu->add_page("Outputs");
        ObjectMultiToggleControl *toggle = new ObjectMultiToggleControl("Enable outputs", true);
        for (unsigned int i = 0 ; i < this->nodes->size() ; i++) {
            BaseOutput *output = this->nodes->get(i);
            //menu->add(new ToggleControl(output->label, output->disabled));
            // options for whether to auto-advance looper/sequencer/beatstep

            MultiToggleItemClass<BaseOutput> *option = new MultiToggleItemClass<BaseOutput> (
                output->label,
                output,
                &BaseOutput::set_enabled,
                &BaseOutput::is_enabled
            );
            toggle->addItem(option);
        }
        menu->add(toggle);
    }

    #include "submenuitem_bar.h"
    #include "menuitems_object.h"
    #include "menuitems_lambda.h"
    #include "menuitems.h"

    #include "mymenu_items/ParameterMenuItems_lowmemory.h"

    void MIDINoteTriggerCountOutput::make_menu_items(Menu *menu, int index) {
        //#ifdef ENABLE_ENVELOPE_MENUS
            char label[40];
            snprintf(label, 40, "MIDINoteOutput %i: %s", index, this->label);
            menu->add_page(label);

            SubMenuItemColumns *sub_menu_item_columns = new SubMenuItemColumns("Options", 2);
            sub_menu_item_columns->add(new ObjectToggleControl("Note mode", this, &MIDINoteTriggerCountOutput::set_note_mode, &MIDINoteTriggerCountOutput::get_note_mode));
            sub_menu_item_columns->add(new DirectNumberControl<int_fast8_t>("Octave", &this->octave, this->octave, (int_fast8_t)0, (int_fast8_t)10));

            menu->add(sub_menu_item_columns);
        //#endif
    }

    void MIDIOutputWrapper::create_menu_items() {
        // controls for cv-to-midi outputs..

        /*LinkedList<FloatParameter*> *parameters = new LinkedList<FloatParameter*>();
        for (int i = 0 ; i < NUM_MIDI_CC_PARAMETERS ; i++) {
            parameters->add(&midi_cc_parameters[i]);
        }*/
        
        char label[MENU_C_MAX];
        for (int i = 0 ; i < NUM_MIDI_CC_PARAMETERS ; i++) {
            snprintf(label, MENU_C_MAX, "CV-to-MIDI: %s", midi_cc_parameters[i].label);
            menu->add_page(label);

            snprintf(label, MENU_C_MAX, "Settings");
            SubMenuItem *bar = new SubMenuItemBar(label);

            snprintf(label, MENU_C_MAX, "Output CC");
            bar->add(new DirectNumberControl<byte>(
                label, 
                &midi_cc_parameters[i].cc_number,
                midi_cc_parameters[i].cc_number,
                0,
                MIDI_MAX_VELOCITY
            ));

            snprintf(label, MENU_C_MAX, "Output MIDI Channel");
            bar->add(new DirectNumberControl<byte>(
                label, 
                &midi_cc_parameters[i].channel,
                midi_cc_parameters[i].channel,
                1,
                16
            ));

            menu->add(bar);

            // todo: use lowmemory controls instead of a full instance of each
            menu->add(midi_cc_parameters[i].makeControls());

            // todo: a little widget that shows the inputs/output value?
        }

        /*menu->add_page("CV-to-MIDI mod");
        for (int i = 0 ; i < NUM_MIDI_CC_PARAMETERS ; i++) {
            menu->add(midi_cc_parameters[i].makeControls());
        }*/

        // todo: probably move this to another more generic 'settings' page
        menu->add_page("MIDI Output");
        LambdaSelectorControl<OUTPUT_TYPE> *output_mode_selector = new LambdaSelectorControl<OUTPUT_TYPE>(
            "DIN output mode", 
            [=](OUTPUT_TYPE a) -> void { this->set_output_mode(a); },
            [=]() -> OUTPUT_TYPE { return this->get_output_mode(); },
            nullptr, 
            true
        );
        for (int i = 0 ; i < sizeof(available_output_types)/sizeof(output_type_t) ; i++) {
            output_mode_selector->add_available_value(available_output_types[i].type_id, available_output_types[i].label);
        }
        menu->add(output_mode_selector);

        #ifdef ENABLE_CLOCK_INPUT_CV
            SelectorControl<uint32_t> *external_cv_ticks_per_pulse_selector = new SelectorControl<uint32_t>("External CV clock: Pulses per tick");
            external_cv_ticks_per_pulse_selector->available_values = external_cv_ticks_per_pulse_values;
            external_cv_ticks_per_pulse_selector->num_values = sizeof(external_cv_ticks_per_pulse_values)/sizeof(uint32_t);
            external_cv_ticks_per_pulse_selector->f_setter = set_external_cv_ticks_per_pulse_values;
            external_cv_ticks_per_pulse_selector->f_getter = get_external_cv_ticks_per_pulse_values;
            menu->add(external_cv_ticks_per_pulse_selector);
        #endif

    }

#endif