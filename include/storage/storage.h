#pragma once

#include "saveloadlib.h"
#include "saveload_settings.h"

#include "sequencer/sequencing.h"
#include "outputs/output_processor.h"

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

    }
};

extern SettingsRoot *settings_root;

#ifdef ENABLE_SCREEN
    void setup_storage_menu();
#endif
