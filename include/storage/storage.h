#pragma once

#include "saveloadlib.h"
#include "saveload_settings.h"

#include "sequencer/sequencing.h"
#include "outputs/output_processor.h"

extern RP2040OutputWrapperClass *output_wrapper; // @@ TODO: find a suitable place to put this in a header

#define PRESET_SLOT_FILEPATH_FORMAT "slots/preset-%i.txt"
#define MAXFILEPATH 32

#define FILE_READ_MODE "r"
#define FILE_WRITE_MODE "w"
#define FILEPATH_CALIBRATION_FORMAT       "calib_volt_src_%i.txt"

void setup_saveloadlib();

class SettingsRoot : public ISaveableSettingHost {
    public:
    SettingsRoot() {
        this->path_segment = "root";
        this->seg_hash = sl_fnv1a_16(this->path_segment);
    }

    virtual void setup_saveable_settings() override {
        // register top nodes of hierarchy
        
        register_child(sequencer);
        register_child(output_processor);
        register_child(output_wrapper);

        // todo: add parameter manage here as well;
        //      it has parameter inputs that need to be saved globally
        // todo: add the MIDI output wrapper here as well, since it has settings that need to be saved globally

    }
};

extern SettingsRoot *settings_root;

#ifdef ENABLE_SCREEN
    void setup_storage_menu();
#endif


bool save_to_slot(int slot);
bool load_from_slot(int slot);