#ifdef ENABLE_CV_INPUT
    #include "outputs/output_voice.h"

    CVChordVoice *cv_chord_output_1;
    CVChordVoice *cv_chord_output_2;
    CVChordVoice *cv_chord_output_3;

    extern RP2040DualMIDIOutputWrapper *output_wrapper;

    #include "cv_input.h"

    void setup_cv_pitch_inputs () {
        cv_chord_output_1 = new CVChordVoice("CV Pitch 1", output_wrapper, parameter_manager->getInputForName("A"), nullptr, 1);
        cv_chord_output_2 = new CVChordVoice("CV Pitch 2", output_wrapper, parameter_manager->getInputForName("B"), nullptr, 1);
        cv_chord_output_3 = new CVChordVoice("CV Pitch 3", output_wrapper, parameter_manager->getInputForName("C"), nullptr, 1);
    }

    void setup_cv_pitch_inputs_menu () {
        cv_chord_output_1->create_menu_items();
        cv_chord_output_2->create_menu_items();
        cv_chord_output_3->create_menu_items();
    }
#endif