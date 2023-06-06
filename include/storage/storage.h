#ifndef STORAGE__INCLUDED
#define STORAGE__INCLUDED

#include <LittleFS.h>

#include "debug.h"
#include "ParameterManager.h"

#include <string>

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

// wrapper class to parse a string out line-by-line
class LineReader {
    String *string = nullptr;
    uint_fast16_t position = 0;

    public:
    LineReader(String *string) {
        this->string = string;
    }
    bool available() {
        return (position < string->length());
    }
    String read_line() {
        uint_fast16_t start = position;
        position = this->string->indexOf('\n', start) + 1;
        return this->string->substring(start, position-2);  // -2 to chop off \r\n
    }
};

bool load_from_slot(int slot) {
    char filename[MAXFILEPATH];
    snprintf(filename, MAXFILEPATH, PRESET_SLOT_FILEPATH_FORMAT, slot);

    //bool return_before_open = false, return_before_read = false, continue_before_parse_1 = false, continue_before_parse_2 = false;
    //if (return_before_open) return false;

    uint32_t millis_at_start_of_load = millis();
    File f = LittleFS.open(filename, FILE_READ_MODE);
    if (f) {
        //messages_log_add("Slot file is " + String(f.size()) + " bytes big.");
        messages_log_add(String("load_from_slot opened ") + String(filename));
        int lines_parsed_count = 0;
        String line;
        /*if (return_before_read) {
            messages_log_add(String("load_from_slot early return took ") + String(millis()-millis_at_start_of_load) + "ms to open file.");
            f.close();
            return false;
        }*/
        String data = f.readString();   // read entire file into memory - much faster than reading a byte at a time like readStringUntil()!
        //messages_log_add("read file to string in " + String((millis()-millis_at_start_of_load)) + "ms, length " + String(data.length()));
        LineReader r(&data);
        while (r.available()) {
            //line = f.readStringUntil('\n');
            line = r.read_line();
            //if (continue_before_parse_1) continue;
            int_fast8_t p = line.indexOf('=');
            String key = line.substring(0, p);
            String value = line.substring(p+1);
            //value.replace("\r","");

            //if (continue_before_parse_2) continue;
            if (!parameter_manager->fast_load_parse_key_value(key,value)) {
                messages_log_add(String("Failed to parse line '") + key + "=" + value);
            }
            lines_parsed_count++;
            //uint32_t millis_now = millis();
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