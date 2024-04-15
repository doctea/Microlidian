#ifndef MENUITEMS_SEQUENCER__H
#define MENUITEMS_SEQUENCER__H

#include "sequencer/Patterns.h"
#include "menuitems.h"
//#include "submenuitem_bar.h"
//#include "menuitems_object_selector.h"
//#define NUM_STEPS 16

#include "sequencer/Euclidian.h"

#include <bpm.h>
#include <clock.h>

class PatternDisplay : public MenuItem {
    public:
        //DeviceBehaviour_Beatstep *behaviour_beatstep = nullptr;
        SimplePattern *target_pattern = nullptr;
        PatternDisplay(const char *label, SimplePattern *target_pattern, bool selectable = true, bool show_header = true) : MenuItem(label, selectable) {
            this->set_pattern(target_pattern);
            this->show_header = show_header;
        }

        void set_pattern(SimplePattern *pattern) {
            this->target_pattern = pattern;
            this->default_fg = pattern->get_colour();
        }

        virtual int display(Coord pos, bool selected, bool opened) override {
            char label_info[MENU_C_MAX];
            if(this->show_header) {
                /*snprintf(label_info, MENU_C_MAX, "%s: Steps=%2i Pulses=%2i Rot=%2i", 
                    this->label, 
                    ((EuclidianPattern*)target_pattern)->get_steps(), 
                    ((EuclidianPattern*)target_pattern)->arguments.pulses, 
                    ((EuclidianPattern*)target_pattern)->arguments.rotation
                );*/
                snprintf(label_info, MENU_C_MAX, "%s: %s", 
                    this->label, 
                    this->target_pattern->get_output_label()
                );
                pos.y = header(label_info, pos, selected, opened);
            }

            if (this->target_pattern==nullptr) {
                tft->println("No pattern selected");
                return tft->getCursorY();
            }

            tft->setCursor(pos.x, pos.y);

            uint16_t base_row = pos.y;
            //static float ticks_per_pixel = (float)LOOP_LENGTH_TICKS / (float)tft->width();

            /*#define STEP_WIDTH 8
            #define STEP_HEIGHT 8
            #define STEP_GAP 2*/
            #define MAX_COLUMNS 16
            //int columns = target_pattern->get_steps();

            static int width_per_cell = tft->width() / MAX_COLUMNS;
            static int STEP_GAP = 2; //width_per_cell - (width_per_cell/4);
            static int STEP_WIDTH = width_per_cell - STEP_GAP;
            static int STEP_HEIGHT = STEP_WIDTH;

            int actual_current_step = target_pattern->get_step_for_tick(ticks);

            for (int step = 0 ; step < target_pattern->get_effective_steps() ; step++) {
                // for every step of sequence

                // first calculate the row, column and on-screen coordinates
                const int row = step / MAX_COLUMNS;
                const int col = step % MAX_COLUMNS;
                const int x = col * (STEP_WIDTH+STEP_GAP);
                const int y = base_row + (row*(STEP_HEIGHT+STEP_GAP));

                const bool is_step_on = target_pattern->query_note_on_for_step(step);
                const uint16_t colour = is_step_on ? 
                    //(actual_current_step == step ? RED      : BLUE) :     // current step active
                    (actual_current_step == step ? C_WHITE  : target_pattern->get_colour()) :     // current step active
                    (actual_current_step == step ? GREY     : tft->halfbright_565(target_pattern->get_colour()));      // current step inactive

                if (is_step_on) {  // yer twisting my melon, man
                    tft->fillRect(x, y, STEP_WIDTH, STEP_HEIGHT, colour);
                } else {        // call the cops
                    tft->drawRect(x, y, STEP_WIDTH, STEP_HEIGHT, colour);
                    //actual->fillRect(x + 1, y + 1, STEP_WIDTH-2, STEP_HEIGHT-2, BLACK); // hollow out the center
                }
            }

            //base_row += (MAX_COLUMNS/STEP_HEIGHT) * ((STEP_HEIGHT + STEP_GAP) * (max(1+(target_pattern->get_steps() / MAX_COLUMNS), 1)));
            //base_row += (1+(target_pattern->get_steps() / MAX_COLUMNS)) * (STEP_HEIGHT + STEP_GAP);
            base_row += max(
                STEP_HEIGHT+STEP_GAP,
                ((target_pattern->get_effective_steps() / MAX_COLUMNS)) * (STEP_HEIGHT + STEP_GAP)
            );
            tft->setCursor(0, base_row);
            return base_row;
            
            //base_row *= (target_pattern->get_steps() / MAX_COLUMNS);

            /*for (int i = 0 ; i < target_pattern->steps ; i++) {
                tft->printf("%02i: ", i);
                tft->printf(get_note_name_c(behaviour_beatstep->sequence->get_pitch_at_step(i)));
                tft->println();
            }*/
            /*pos.y = tft->getCursorY();

            return pos.y; // + 40; //row + 12; //pos.y + (first_found - last_found); //127;  
            */
        }
};

#endif