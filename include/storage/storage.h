#ifdef ENABLE_STORAGE

#pragma once

#include "saveloadlib.h"
#include "saveload_settings.h"

#include "conductor.h"
#include "sequencer/sequencing.h"
#include "outputs/output_processor.h"
#ifdef ENABLE_ARRANGER
    #include "arranger.h"
#endif

extern bool actively_saving;

#ifdef ENABLE_CV_INPUT
    #include "outputs/output_voice.h"
    #include "cv_input.h"
#endif

#ifdef ENABLE_PARAMETERS
    #include "ParameterManager.h"
#endif

extern RP2040OutputWrapperClass *output_wrapper; // @@ TODO: find a suitable place to put this in a header

#define SYSTEM_SETTINGS_FILEPATH "system.txt"

#define PRESET_SLOT_FILEPATH_FORMAT "slots/preset-%i.txt"
#define SNAPSHOT_SLOT_FILEPATH_FORMAT "snapshots/snapshot-%i.txt"
#define MAXFILEPATH 32

#define FILE_READ_MODE "r"
#define FILE_WRITE_MODE "w"
#define FILEPATH_CALIBRATION_FORMAT "calib_volt_src_%i.txt"

void setup_saveloadlib();

void debug_print_file(const char* filepath);
void queue_file_output(const char* filepath);
bool process_queued_file_output();

#ifdef ENABLE_TESTSAVELOAD
    #include "saveload_test.h"
#endif

class SettingsRoot : public SHDynamic<10, 4> {  // all top-level hosts as children
    public:
    SettingsRoot() {
        this->set_path_segment("root");
        this->seg_hash = sl_fnv1a_16(this->path_segment);
    }

    virtual void setup_saveable_settings() override {
        // register top nodes of hierarchy

        #ifdef ENABLE_TESTSAVELOAD
            // test object
            test_object = new TestSaveableObject();
            register_child(test_object);
        #endif
        
        register_child(sequencer);
        register_child(output_processor);
        register_child(output_wrapper);
        register_child(conductor);

        #ifdef ENABLE_ARRANGER
            if (arranger != nullptr) {
                register_child(arranger);
            }
        #endif

        // CV Pitch settings
        #ifdef ENABLE_CV_INPUT
            register_child(cv_chord_output_1);
            register_child(cv_chord_output_2);
            register_child(cv_chord_output_3);

            // add the midi_cc_parameters, too!
            // these are not strictly only CV_INPUT things, as can also mix internal LFOs and Envelopes etc
            // @@TODO: this will need Parameter converting to new ISaveableSetting system as well
            // for (int i = 0 ; i < NUM_MIDI_CC_PARAMETERS ; i++) {
            //     register_child(midi_cc_parameters[i]);
            // }
        #endif

        // todo: add parameter manager here as well;
        //      it has parameter inputs that need to be saved globally
        //      anything else..?
        #ifdef ENABLE_PARAMETERS
            register_child(parameter_manager);
        #endif
    }
};

extern SettingsRoot *settings_root;

#ifdef ENABLE_SCREEN
    void setup_storage_menu();
#endif

void save_system_settings();
void load_system_settings();
bool save_to_slot(int slot);
bool load_from_slot(int slot);

#endif