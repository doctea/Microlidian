#include "sequencer/Patterns.h"
#include "menuitems.h"
//#include "submenuitem_bar.h"
//#include "menuitems_object_selector.h"
//#define NUM_STEPS 16

#include "sequencer/Euclidian.h"

#include <bpm.h>
#include <clock.h>

#include "midi_helpers.h"
#include "scales.h"

class ScaleMenuItem : public MenuItem {
    public:

        int8_t scale_number = 0;
        int8_t root_note = SCALE_ROOT_A;

        ScaleMenuItem(const char *label) : MenuItem(label) {}

        virtual int display(Coord pos, bool selected, bool opened) override {
            char label[MENU_C_MAX];
            snprintf(label, MENU_C_MAX, 
                "%s: %-3i => %3s %s", this->label,  //  [%i]
                this->root_note, 
                (char*)get_note_name_c(root_note), 
                scales[scale_number].label
                //, scale_number
            );
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

        int mode = 0;

        virtual bool button_select () {
            mode = !mode;
            if (mode) {
                menu_set_last_message("Toggled to SCALE", YELLOW);
            } else {
                menu_set_last_message("Toggled to ROOT", YELLOW);
            }
            return go_back_on_select;
        }

        virtual bool knob_left() override {
            if (mode==0) {
                root_note--;
                if (root_note < 0) root_note = 12;
            } else {
                scale_number--;
                if (scale_number<0 || scale_number>=NUMBER_SCALES)
                    scale_number = NUMBER_SCALES-1;
            }
            return true;
        }
        virtual bool knob_right() override {
            if (mode==0) {
                root_note++;
                root_note %= 12;
            } else {
                scale_number++;
                if (scale_number >= NUMBER_SCALES)
                    scale_number = 0;
            }
            return true;
        }
};