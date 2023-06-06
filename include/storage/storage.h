#ifndef STORAGE__INCLUDED
#define STORAGE__INCLUDED

#include <LittleFS.h>

#include "debug.h"
#include "ParameterManager.h"

void setup_storage() {
    Serial.println("setup_storage()...");
    if (!LittleFS.begin()) {
        Serial.print("Filesystem not formatted - formatting now..");
        LittleFS.format();
        Serial.println("Formatted!");
    } else {
        Serial.println("Initialised OK so must be formatted!");
    }

    /*File f = LittleFS.open("test_file.txt", "r");
    f.setTimeout(0);
    if (!f) {
        Serial.println("Couldn't open test file!");
    } else {
        int c = 0;
        while (f.available()) {
            String r = f.readStringUntil('\n');
            Serial.printf("%i: Read back line '%s'!\n", c++, r.c_str());
        }
        f.close();
    }
    f = LittleFS.open("test_file.txt", "a");
    f.seek(0, SeekEnd);
    f.print("Booted+wrote!\n");
    f.close();*/
   
}

#define PRESET_SLOT_FILEPATH_FORMAT "slots/preset-%i.txt"
#define MAXFILEPATH 32

#define FILE_READ_MODE "r"
#define FILE_WRITE_MODE "w"
#define FILEPATH_CALIBRATION_FORMAT       "calib_volt_src_%i.txt"

bool load_from_slot(int slot) {
    uint32_t millis_at_start_of_load = millis();
    char filename[MAXFILEPATH];
    snprintf(filename, MAXFILEPATH, PRESET_SLOT_FILEPATH_FORMAT, slot);
    File f = LittleFS.open(filename, FILE_READ_MODE);
    if (f) {
        messages_log_add(String("load_from_slot opened ") + String(filename));
        int lines_parsed_count = 0;
        String line;
        while (f.available()) {
            line = f.readStringUntil('\n');
            String key = line.substring(0, line.indexOf("="));
            String value = line.substring(line.indexOf("=")+1);
            value.replace("\r","");

            if (!parameter_manager->fast_load_parse_key_value(key,value)) {
                messages_log_add(String("Failed to parse line '") + String(key) + "=" + String(value));
            }
            lines_parsed_count++;
        }
        f.close();
        messages_log_add(String("load_from_slot took ") + String(millis()-millis_at_start_of_load) + "ms to parse " + String(lines_parsed_count) + " lines.");
        return true;
    } else {
        messages_log_add(String("load_from_slot failed to open ") + String(filename));
        return false;
    }
}

bool save_to_slot(int slot) {
    char filename[MAXFILEPATH];
    snprintf(filename, MAXFILEPATH, PRESET_SLOT_FILEPATH_FORMAT, slot);
    File f = LittleFS.open(filename, FILE_WRITE_MODE);
    if (f) {
        messages_log_add(String("save_to_slot opened ") + String(filename));
        LinkedList<String> *lines = new LinkedList<String> ();

        // get all the parameter mapping values and save them to file
        parameter_manager->add_all_save_lines(lines);
        const unsigned int size = lines->size();
        for (unsigned int i = 0 ; i < size ; i++) {
            f.println(lines->get(i));
        }

        f.close();
        lines->clear();
        delete lines;
        return false;
    } else {
        messages_log_add(String("save_to_slot failed to open ") + String(filename));
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

void setup_storage_menu() {
    menu->add_page("Storage");

    menu->add(new ActionConfirmItem("Save to slot 0",   &save_to_slot_0));
    menu->add(new ActionConfirmItem("Load from slot 0", &load_from_slot_0));
    menu->add(new ActionConfirmItem("Save to slot 1",   &save_to_slot_1));
    menu->add(new ActionConfirmItem("Load from slot 1", &load_from_slot_1));
    menu->add(new ActionConfirmItem("Save to slot 2",   &save_to_slot_2));
    menu->add(new ActionConfirmItem("Load from slot 2", &load_from_slot_2));
}

#endif