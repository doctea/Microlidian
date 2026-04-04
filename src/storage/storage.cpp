#include "storage/storage.h"
#include "saveload_settings.h"

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

void setup_storage_menu() {

    menu->remember_opened_page(
        menu->add_page("Storage")
    );

    menu->add(new MenuItem("placeholder for storage"));

    /*typedef void(*function)();
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
    }*/
}