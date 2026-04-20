#ifdef ENABLE_CV_INPUT
    #include "inputs/cv_pitch_input.h"

    CVPitchTrigger *cv_pitch_input_1;
    CVPitchTrigger *cv_pitch_input_2;
    CVPitchTrigger *cv_pitch_input_3;

    #include "cv_input.h"

    void setup_cv_pitch_inputs(BaseOutputProcessor *output_processor) {

        // @@TODO: map the appropriate sequencer outputs and get available_outputs
        cv_pitch_input_1 = new CVPitchTrigger("CV Pitch 1", output_processor->get_output_for_label("Chords"), output_processor->get_available_outputs(), parameter_manager->getInputForName("A"), nullptr);
        cv_pitch_input_2 = new CVPitchTrigger("CV Pitch 2", output_processor->get_output_for_label("Chords"), output_processor->get_available_outputs(), parameter_manager->getInputForName("B"), nullptr);
        cv_pitch_input_3 = new CVPitchTrigger("CV Pitch 3", output_processor->get_output_for_label("Chords"), output_processor->get_available_outputs(), parameter_manager->getInputForName("C"), nullptr);
    }

    void setup_cv_pitch_inputs_menu() {
        cv_pitch_input_1->create_menu_items();
        cv_pitch_input_2->create_menu_items();
        cv_pitch_input_3->create_menu_items();
    }
#endif
