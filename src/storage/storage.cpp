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

    Serial.println("setup_saveloadlib() finished!");

    // for debug, try opening the file and print its contents to serial:
    debug_print_file("/slots/preset-0.txt");
   
    // // for debug, print the whole settings tree to serial
    // although this works to print out the settings tree,
    // it causes the display to fail to update afterwards, so commenting out for now
    //sl_print_tree_to_print(settings_root, Serial, 10);
}


bool save_to_slot(int slot) {

    char filename[MAXFILEPATH];
    snprintf(filename, MAXFILEPATH, PRESET_SLOT_FILEPATH_FORMAT, slot);

    // acquire_lock(); // @@TODO: see if this is actually necessary to prevent display updates while loading, or if we can get away with just disabling interrupts around the critical sections of the load code instead

    Serial.printf("Saving to slot %i (file %s)...\n", slot, filename); Serial.flush();
    uint32_t micros_at_start_of_save = micros();
    bool status = sl_save_to_file(settings_root, filename);
    uint32_t micros_at_end_of_save = micros();
    Serial.printf("Finished saving to slot %i (file %s) in %u us\n", slot, filename, (micros_at_end_of_save - micros_at_start_of_save)); Serial.flush();

    // release_lock();// @@TODO: see if this is actually necessary to prevent display updates while loading, or if we can get away with just disabling interrupts around the critical sections of the load code instead

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
    bool status = sl_load_from_file(filename);

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
            DualMenuItem *submenuitem = new DualMenuItem(label);

            snprintf(label, MENU_C_MAX, "Load %i", i);
            submenuitem->add(new ActionConfirmItem(label, functions[i].load, false));

            snprintf(label, MENU_C_MAX, "Save %i", i);
            submenuitem->add(new ActionConfirmItem(label, functions[i].save, false));

            menu->add(submenuitem);
        }

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
                        acquire_lock(); // @@TODO: see if this is actually necessary to prevent display updates while loading, or if we can get away with just disabling interrupts around the critical sections of the load code instead
                        ATOMIC() {
                            debug_print_file(filename);
                        }
                        release_lock(); // @@TODO: see if this is actually necessary to prevent display updates while loading, or if we can get away with just disabling interrupts around the critical sections of the load code instead
                    }
                )
            );
        }
        menu->add(debug_bar);

        // option to dump the whole settings tree to serial for debugging
        menu->add(new ActionConfirmItem("Dump settings tree to serial", []() { 
            sl_print_tree_to_print(settings_root, Serial, 10); }, false)
        );

    }
#endif
