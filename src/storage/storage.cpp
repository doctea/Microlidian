#include "storage/storage.h"
#include "saveload_settings.h"

#include "core_safe.h"

SettingsRoot *settings_root;

void setup_saveloadlib() {

    settings_root = new SettingsRoot();

    if (!LittleFS.begin()) {
        Debug_print("Filesystem not formatted - formatting now..");
        messages_log_add("LittleFS: Filesystem not formatted - formatting now..");
        LittleFS.format();
        Debug_println("Formatted!");
        messages_log_add("LittleFS: Formatted!");
    } else {
        //Debug_println("Initialised OK so must be formatted!");
        messages_log_add("LittleFS: Initialised OK so must be formatted!");
    }

    Serial.printf("Before sl_register_root(), free RAM is %u\n", freeRam());
    sl_register_root(settings_root);
    Serial.printf("After sl_register_root(), free RAM is %u\n", freeRam());

    // cascade setup of all saveable settings starting from root
    Serial.printf("Before sl_setup_all(), free RAM is %u\n", freeRam());
    sl_setup_all(settings_root);
    Serial.printf("After sl_setup_all(), free RAM is %u\n", freeRam());
   
    // // for debug, print the whole settings tree to serial
    // although this works to print out the settings tree,
    // it causes the display to fail to update afterwards, so commenting out for now
    //sl_print_tree_to_print(settings_root, Serial, 10);
}


bool save_to_slot(int slot) {

    char filename[MAXFILEPATH];
    snprintf(filename, MAXFILEPATH, PRESET_SLOT_FILEPATH_FORMAT, slot);

    acquire_lock();

    Serial.printf("Saving to slot %i (file %s)...\n", slot, filename); Serial.flush();
    bool status = sl_save_to_file(settings_root, filename);

    release_lock();

    if (status) {
        messages_log_add(String("Saved to slot ") + String(slot));
        //Serial.printf("Saved to slot %i (file %s)!\n", slot, filename); Serial.flush();
        return true;
    } else {
        messages_log_add(String("Failed to save to slot ") + String(slot));
        //Serial.printf("Failed to save to slot %i (file %s)!\n", slot, filename); Serial.flush();
        return false;
    }
}

bool load_from_slot(int slot) {

    char filename[MAXFILEPATH];
    snprintf(filename, MAXFILEPATH, PRESET_SLOT_FILEPATH_FORMAT, slot);

    if (!LittleFS.exists(filename)) {
        messages_log_add(String("File does not exist: ") + String(filename));
        //Serial.printf("File does not exist: %s\n", filename); Serial.flush();
        return false;
    }

    //Serial.printf("Loading from slot %i (file %s)...\n", slot, filename); Serial.flush();
    bool status = sl_load_from_file(filename);

    if (status) {
        messages_log_add(String("Loaded from slot ") + String(slot));
        //Serial.printf("Loaded from slot %i (file %s)!\n", slot, filename); Serial.flush();
        return true;
    } else {
        messages_log_add(String("Exists, but failed to load from slot ") + String(slot));
        //Serial.printf("Failed to load from slot %i (file %s)!\n", slot, filename); Serial.flush();
        return false;
    }
}


void save_to_slot_0() {
    //return 
    save_to_slot(0);
}
void load_from_slot_0() {
    //return
    load_from_slot(0);
}
void save_to_slot_1() {
    //return
    save_to_slot(1);
}
void load_from_slot_1() {
    //return
    load_from_slot(1);
}
void save_to_slot_2() {
    //return
    save_to_slot(2);
}
void load_from_slot_2() {
    //return
    load_from_slot(2);
}

#include "mymenu.h"

#ifdef ENABLE_SCREEN
    void setup_storage_menu() {
        menu->remember_opened_page(
            menu->add_page("Storage")
        );

        typedef void(*function)();
        struct functions_t {
            function save;
            function load;
        };
        functions_t functions[] = {
            { &save_to_slot_0, &load_from_slot_0 },
            { &save_to_slot_1, &load_from_slot_1 },
            { &save_to_slot_2, &load_from_slot_2 }
        };

        for (int i = 0 ; i < sizeof(functions)/sizeof(functions_t) ; i++) {
            char label[MENU_C_MAX];
            snprintf(label, MENU_C_MAX, "Preset slot %i", i);
            DualMenuItem *submenuitem = new DualMenuItem(label);

            submenuitem->add(new ActionConfirmItem("Load", functions[i].load, false));
            submenuitem->add(new ActionConfirmItem("Save", functions[i].save, false));

            menu->add(submenuitem);
        }
    }
#endif
