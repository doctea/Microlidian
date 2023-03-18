#ifndef STORAGE__INCLUDED
#define STORAGE__INCLUDED

#include <LittleFS.h>

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

#endif