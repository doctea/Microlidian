#ifdef ENABLE_CV_INPUT
    #include "outputs/output_voice.h"

    CVChordVoice *cv_chord_output_1;
    CVChordVoice *cv_chord_output_2;
    CVChordVoice *cv_chord_output_3;

    extern RP2040DualMIDIOutputWrapper *output_wrapper;

    #include "cv_input.h"

    void setup_cv_pitch_inputs(BaseOutputProcessor *output_processor) {

        // @@TODO: map the appropriate sequencer outputs and get available_outputs
        cv_chord_output_1 = new CVChordVoice("CV Pitch 1", output_processor->get_output_for_label("Chords"), output_processor->get_available_outputs(), parameter_manager->getInputForName("A"), nullptr);
        cv_chord_output_2 = new CVChordVoice("CV Pitch 2", output_processor->get_output_for_label("Chords"), output_processor->get_available_outputs(), parameter_manager->getInputForName("B"), nullptr);
        cv_chord_output_3 = new CVChordVoice("CV Pitch 3", output_processor->get_output_for_label("Chords"), output_processor->get_available_outputs(), parameter_manager->getInputForName("C"), nullptr);
    }

    void setup_cv_pitch_inputs_menu () {
        cv_chord_output_1->create_menu_items();
        cv_chord_output_2->create_menu_items();
        cv_chord_output_3->create_menu_items();
    }
#endif