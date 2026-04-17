
#ifdef ENABLE_STORAGE

#include "storage/storage.h"
#include "saveload_settings.h"

#include "saveload_test.h"

#include "core_safe.h"
#include <profiling.h>

SettingsRoot *settings_root;

#ifdef ENABLE_TESTSAVELOAD
    TestSaveableObject *test_object;
#endif

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

    // for debug, try opening the file and print its contents to serial:
    // acquire_lock();
    // debug_print_file("/slots/preset-0.txt");
    // release_lock();
    
    sl_validate_tree(settings_root, Serial);  
    // sl_print_tree_to_print(settings_root, Serial);
    
    Serial.println("setup_saveloadlib() finished!");
    
}

// save/load system settings
void save_system_settings() {
    acquire_lock();
    bool status = sl_save_to_file(settings_root, SYSTEM_SETTINGS_FILEPATH, SL_SCOPE_SYSTEM);
    release_lock();
    if (status) {
        messages_log_add("Saved system settings");
    } else {
        messages_log_add("Failed to save system settings");
    }
}
void load_system_settings() {
    if (!LittleFS.exists(SYSTEM_SETTINGS_FILEPATH)) {
        messages_log_add(String("System settings file does not exist: ") + String(SYSTEM_SETTINGS_FILEPATH));
        return;
    }
    acquire_lock();
    bool status = sl_load_from_file(SYSTEM_SETTINGS_FILEPATH, SL_SCOPE_SYSTEM);
    release_lock();
}

bool save_to_slot(int slot) {

    char filename[MAXFILEPATH];
    snprintf(filename, MAXFILEPATH, PRESET_SLOT_FILEPATH_FORMAT, slot);

    // acquire_lock() prevents core 1 from being interrupted mid-transaction by LittleFS flash writes
    acquire_lock();

    Serial.printf("Saving to slot %i (file %s)...\n", slot, filename); Serial.flush();
    uint32_t micros_at_start_of_save = micros();
    bool status = sl_save_to_file(settings_root, filename, SL_SCOPE_SCENE | SL_SCOPE_PROJECT);
    uint32_t micros_at_end_of_save = micros();
    Serial.printf("Finished saving to slot %i (file %s) in %u us\n", slot, filename, (micros_at_end_of_save - micros_at_start_of_save)); Serial.flush();

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

    
    acquire_lock(); // @@TODO: see if this is actually necessary to prevent display updates while loading, or if we can get away with just disabling interrupts around the critical sections of the load code instead

    //Serial.printf("Loading from slot %i (file %s)...\n", slot, filename); Serial.flush();
    bool status = sl_load_from_file(filename, SL_SCOPE_SCENE | SL_SCOPE_PROJECT);

    release_lock(); // @@TODO: see if this is actually necessary to prevent display updates while loading, or if we can get away with just disabling interrupts around the critical sections of the load code instead

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


bool save_to_snapshot(int slot) {
    
    char filename[MAXFILEPATH];
    snprintf(filename, MAXFILEPATH, SNAPSHOT_SLOT_FILEPATH_FORMAT, slot);

    // acquire_lock() prevents core 1 from being interrupted mid-transaction by LittleFS flash writes
    acquire_lock();

    Serial.printf("Saving to snapshot slot %i (file %s)...\n", slot, filename); Serial.flush();
    uint32_t micros_at_start_of_save = micros();
    bool status = sl_save_to_file(settings_root, filename, SL_SCOPE_SNAPSHOT);
    uint32_t micros_at_end_of_save = micros();
    Serial.printf("Finished saving to snapshot slot %i (file %s) in %u us\n", slot, filename, (micros_at_end_of_save - micros_at_start_of_save)); Serial.flush();

    release_lock();

    if (status) {
        messages_log_add(String("Saved to snapshot slot ") + String(slot));
        //Serial.printf("Saved to snapshot slot %i (file %s)!\n", slot, filename); Serial.flush();
        return true;
    } else {
        messages_log_add(String("Failed to save to snapshot slot ") + String(slot));
        //Serial.printf("Failed to save to snapshot slot %i (file %s)!\n", slot, filename); Serial.flush();
        return false;
    }
}

bool load_from_snapshot(int slot) {

    char filename[MAXFILEPATH];
    snprintf(filename, MAXFILEPATH, SNAPSHOT_SLOT_FILEPATH_FORMAT, slot);

    acquire_lock();

    Serial.printf("Loading from snapshot slot %i (file %s)...\n", slot, filename); Serial.flush();
    bool status = sl_load_from_file(filename, SL_SCOPE_SNAPSHOT);
    release_lock();

    if (status) {
        messages_log_add(String("Loaded from snapshot slot ") + String(slot));
        //Serial.printf("Loaded from snapshot slot %i (file %s)!\n", slot, filename); Serial.flush();
        return true;
    } else {
        messages_log_add(String("Failed to load from snapshot slot ") + String(slot));
        //Serial.printf("Failed to load from snapshot slot %i (file %s)!\n", slot, filename); Serial.flush();
        return false;
    }
}


static char queued_filename[MAXFILEPATH] = "";
// use "$$$savetree" as a special case to dump the whole sttings tree to serial instead of a file
void queue_file_output(const char *filename) {
    strncpy(queued_filename, filename, MAXFILEPATH);
    queued_filename[MAXFILEPATH-1] = '\0'; // ensure null termination
}

inline void reset_queuedfile() {
    queued_filename[0] = '\0';
}

// process the queued file output, if any, by printing the file contents to serial.  
// special case given if filename is "$$$savetree", the whole settings tree will be printed instead of a file.
// Returns true if a file was output, false if no file was queued or if still outputting a previous file.
bool process_queued_file_output() {
    static bool is_outputting = false;

    if (is_outputting) 
        return false; // still outputting previous file, don't start a new one yet

    char filename[MAXFILEPATH];
    strncpy(filename, queued_filename, MAXFILEPATH);
    filename[MAXFILEPATH-1] = '\0'; // ensure null termination

    if (filename[0] == '\0') 
        return false; // no file queued

    if (strcmp(filename, "$$$savetree") == 0) {
        acquire_lock();
        Serial.println("Dumping settings tree to serial...");
        is_outputting = true; // set this early to prevent any other queued outputs from starting while we're still outputting this one
        sl_print_tree_to_print(settings_root, Serial, 10);
        sl_validate_tree(settings_root, Serial);  // print validation warnings at the end
        is_outputting = false;
        reset_queuedfile(); // clear the queue
        release_lock();
        return true;
    }

    if (strcmp(filename, "$$$profile") == 0) {
        // Dump profiling counters — only meaningful when ENABLE_PROFILING is set.
        // Trigger via queue_file_output("$$$profile") or the "Dump profile" menu item.
        acquire_lock();
        profile_print_all(Serial);
        reset_queuedfile();
        release_lock();
        return true;
    }

    if (strcmp(filename, "$$$profilereset") == 0) {
        // Reset all profiling counters so you get a clean window of samples.
        acquire_lock();
        profile_reset_all();
        Serial.println("Profile counters reset.");
        reset_queuedfile();
        release_lock();
        return true;
    }

    is_outputting = true;
    Serial.printf("Outputting file %s to serial...\n", filename); Serial.flush();
    debug_print_file(filename);
    reset_queuedfile(); // clear the queue
    is_outputting = false;
    return true;
}

void save_to_slot_0() {     save_to_slot(0);}
void save_to_slot_1() {     save_to_slot(1);}
void save_to_slot_2() {     save_to_slot(2);}
void save_to_slot_3() {     save_to_slot(3);}
void save_to_slot_4() {     save_to_slot(4);}
void save_to_slot_5() {     save_to_slot(5);}
void save_to_slot_6() {     save_to_slot(6);}
void save_to_slot_7() {     save_to_slot(7);}

void load_from_slot_0() {   load_from_slot(0);}
void load_from_slot_1() {   load_from_slot(1);}
void load_from_slot_2() {   load_from_slot(2);}
void load_from_slot_3() {   load_from_slot(3);}
void load_from_slot_4() {   load_from_slot(4);}
void load_from_slot_5() {   load_from_slot(5);}
void load_from_slot_6() {   load_from_slot(6);}
void load_from_slot_7() {   load_from_slot(7);}

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
            { &save_to_slot_2, &load_from_slot_2 },
            { &save_to_slot_3, &load_from_slot_3 },
            { &save_to_slot_4, &load_from_slot_4 },
            { &save_to_slot_5, &load_from_slot_5 },
            { &save_to_slot_6, &load_from_slot_6 },
            { &save_to_slot_7, &load_from_slot_7 }
        };

        for (int i = 0 ; i < sizeof(functions)/sizeof(functions_t) ; i++) {
            char label[MENU_C_MAX];
            snprintf(label, MENU_C_MAX, "Preset slot %i", i);
            DualMenuItem *submenuitem = new DualMenuItem(label, false);

            snprintf(label, MENU_C_MAX, "Load %i", i);
            submenuitem->add(new ActionConfirmItem(label, functions[i].load, false));

            snprintf(label, MENU_C_MAX, "Save %i", i);
            submenuitem->add(new ActionConfirmItem(label, functions[i].save, false));

            menu->add(submenuitem);
        }

        DualMenuItem *system_settings_bar = new DualMenuItem("System settings");
        system_settings_bar->add(new ActionConfirmItem("Load", &load_system_settings, false));
        system_settings_bar->add(new ActionConfirmItem("Save", &save_system_settings, false));
        menu->add(system_settings_bar);

        for (int i = 0 ; i < 8 ; i++) {
            char label[MENU_C_MAX];
            snprintf(label, MENU_C_MAX, "Snapshot slot %i", i);
            DualMenuItem *submenuitem = new DualMenuItem(label);

            snprintf(label, MENU_C_MAX, "Load %i", i);
            submenuitem->add(new LambdaActionConfirmItem(label, [=]() { load_from_snapshot(i); }));

            snprintf(label, MENU_C_MAX, "Save %i", i);
            submenuitem->add(new LambdaActionConfirmItem(label, [=]() { save_to_snapshot(i); }));

            menu->add(submenuitem);
        }

        // todo: debug stuff to debug page
        // options to dump files to serial for debugging
        SubMenuItemBar *debug_bar = new SubMenuItemBar("Dump file to serial", true, true);
        for (int i = 0 ; i < sizeof(functions)/sizeof(functions_t) ; i++) {
            char label[MENU_C_MAX];
            snprintf(label, MENU_C_MAX, "%i", i);
            debug_bar->add(
                new LambdaActionConfirmItem(
                    label, 
                    [=]() {
                        char filename[MAXFILEPATH];
                        snprintf(filename, MAXFILEPATH, PRESET_SLOT_FILEPATH_FORMAT, i);
                        queue_file_output(filename);
                    }
                )
            );
        }
        debug_bar->add(
            new LambdaActionConfirmItem(
                "System", 
                []() {
                    queue_file_output(SYSTEM_SETTINGS_FILEPATH);
                }
            )
        );
        menu->add(debug_bar);

        // option to dump the whole settings tree to serial for debugging
        menu->add(new ActionConfirmItem("Dump settings tree to serial", []() {
            queue_file_output("$$$savetree");
        }));

        #ifdef ENABLE_PROFILING
            // profiling — only meaningful when ENABLE_PROFILING is defined in build_flags
            menu->add(new ActionConfirmItem("Dump profile to serial", []() {
                queue_file_output("$$$profile");
            }));
            menu->add(new ActionConfirmItem("Reset profile counters", []() {
                queue_file_output("$$$profilereset");
            }));
        #endif

    }
#endif

#endif