#include "sequencer/Patterns.h"
#include "menuitems.h"
//#include "submenuitem_bar.h"
//#include "menuitems_object_selector.h"
//#define NUM_STEPS 16

#include "sequencer/Euclidian.h"

#include <bpm.h>
#include <clock.h>

#include "midi_helpers.h"

class ScaleMenuItem : public MenuItem {
    public:

        byte scale_number = 0;
        int8_t root_note = SCALE_ROOT_A;

        ScaleMenuItem(const char *label) : MenuItem(label) {}

        virtual int display(Coord pos, bool selected, bool opened) override {
            char label[MENU_C_MAX];
            snprintf(label, MENU_C_MAX, "%s: %-3i => %3s", this->label, this->root_note, (char*)get_note_name_c(root_note));
            pos.y = header(label, pos, selected, opened);
            //tft->printf("Root note %-3i => %3s\n", (int)this->root_note, (char*)get_note_name_c(root_note));
            for (int i = 0 ; i < 24 ; i++) {
                if (i==12)
                    tft->setCursor(tft->width()/2, pos.y);
                if (i>=12) 
                    tft->setCursor(tft->width()/2, tft->getCursorY());
                byte quantised_note = quantise_pitch(root_note + i, this->root_note, this->scale_number);
                if (quantised_note!=root_note + i) {
                    colours(false, RED);
                } else {
                    colours(false, GREEN);
                }
                //tft->printf("%s => %s\n", get_note_name_c(i), get_note_name_c(quantised_note));
                tft->printf("%-3i: ", i);
                tft->printf("%-3s", (char*)get_note_name_c(root_note + i));
                tft->print(" => ");
                tft->printf("%-3s", (char*)get_note_name_c(quantised_note));
                //if (quantised_note!=i) tft->print(" - quantised!");
                tft->println();
            }
            return tft->getCursorY();
        }

        virtual bool knob_left() override {
            root_note--;
            if (root_note < 0) root_note = 12;
            return true;
        }
        virtual bool knob_right() override {
            root_note++;
            //if (root_note > 96) root_note = 0;
            root_note %= 12;
            return true;
        }
};