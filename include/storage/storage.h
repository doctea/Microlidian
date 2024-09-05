#ifndef STORAGE__INCLUDED
#define STORAGE__INCLUDED

#include <LittleFS.h>

#include "debug.h"
#include "ParameterManager.h"
#include "outputs/output.h"

#include <string>

extern RP2040DualMIDIOutputWrapper *output_wrapper;

void setup_storage() {
    Debug_println("setup_storage()...");
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
        #ifdef USE_UCLOCK
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) 
        #endif
        {
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
                if (output_wrapper->load_parse_key_value(key,value)) {
                    // succeeded loading via output_wrapper MIDIOutputWrapper
                    //Serial.printf(">>output_wrapper handled line\t%s\n", line.c_str());
                } else if (parameter_manager->fast_load_parse_key_value(key,value)) {
                    // succeeded loading via parameter_manager ..
                    //Serial.printf(">>parameter_manager handled line\t%s\n", line.c_str());
                } else {
                    messages_log_add(String("!!Failed to parse line\t'") + key + "=" + value);
                }
                lines_parsed_count++;
                //uint32_t millis_now = millis();
            }
            f.close();
            messages_log_add(String("load_from_slot took ") + String(millis()-millis_at_start_of_load) + "ms to parse " + String(lines_parsed_count) + " lines.");
        }
        return true;
    } else {
        messages_log_add(String("load_from_slot failed to open ") + String(filename));
        return false;
    }
}

bool save_to_slot(int slot) {
    char filename[MAXFILEPATH];
    snprintf(filename, MAXFILEPATH, PRESET_SLOT_FILEPATH_FORMAT, slot);
    //Serial.printf("save_to_slot starting save process, freeRam is %u\n", freeRam()); Serial_flush();
    File f = LittleFS.open(filename, FILE_WRITE_MODE);
    if (f) {
        //Serial.printf("save_to_slot file opened: freeRam is %u\n", freeRam()); Serial_flush();
        #ifdef USE_UCLOCK
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) 
        #endif
        {
            messages_log_add(String("save_to_slot opened ") + String(filename));
            LinkedList<String> *lines = new LinkedList<String> ();
            //Serial.printf("save_to_slot created LinkedList: freeRam is %u\n", freeRam()); Serial_flush();

            // get all the parameter mapping values
            parameter_manager->add_all_save_lines(lines);

            // get MIDIOutputWrapper lines
            output_wrapper->add_all_save_lines(lines);

            // save them to file
            const uint_fast16_t size = lines->size();
            for (uint_fast16_t i = 0 ; i < size ; i++) {
                //Serial.printf("Saving line: %s\n", lines->get(i).c_str());
                f.println(lines->get(i));
            }

            //Serial.printf("save_to_slot file written: freeRam is %u\n", freeRam()); Serial_flush();
            f.close();
            //Serial.printf("save_to_slot closed file: freeRam is %u\n", freeRam()); Serial_flush();
            lines->clear();
            //Serial.printf("save_to_slot clear() lines: freeRam is %u\n", freeRam()); Serial_flush();
            delete lines;
            //Serial.printf("save_to_slot deleted lines: freeRam is %u\n", freeRam()); Serial_flush();
        }
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

#endif
