#include "sequencer/Patterns.h"
#include "menuitems.h"
//#include "submenuitem_bar.h"
//#include "menuitems_object_selector.h"
//#define NUM_STEPS 16

#include <bpm.h>
#include <clock.h>

class PatternDisplay : public MenuItem {
    public:
        //DeviceBehaviour_Beatstep *behaviour_beatstep = nullptr;
        SimplePattern *target_pattern = nullptr;
        PatternDisplay(const char *label, SimplePattern *target_pattern) : MenuItem(label) {
            this->set_pattern(target_pattern);
        }

        void set_pattern(SimplePattern *pattern) {
            this->target_pattern = pattern;
        }

        virtual int display(Coord pos, bool selected, bool opened) override {
            pos.y = header(label, pos, selected, opened);

            if (this->target_pattern==nullptr) {
                tft->println("No pattern selected");
                return tft->getCursorY();
            }

            tft->setCursor(pos.x, pos.y);

            uint16_t base_row = pos.y;
            //static float ticks_per_pixel = (float)LOOP_LENGTH_TICKS / (float)tft->width();

            // we're going to use direct access to the underlying Adafruit library here
            const DisplayTranslator_Bodmer *tft2 = (DisplayTranslator_Bodmer*)tft;
            TFT_eSPI *actual = tft2->tft;

            /*#define STEP_WIDTH 8
            #define STEP_HEIGHT 8
            #define STEP_GAP 2*/
            #define COLUMNS 16

            int width_per_cell = tft->width() / COLUMNS;
            int STEP_GAP = 2; //width_per_cell - (width_per_cell/4);
            int STEP_WIDTH = width_per_cell - STEP_GAP;
            int STEP_HEIGHT = STEP_WIDTH;

            for (int i = 0 ; i < target_pattern->steps ; i++) {
                int row = ((i / COLUMNS));
                int col = i % COLUMNS;

                int x = col * (STEP_WIDTH+STEP_GAP);
                int y = base_row + (row*(STEP_HEIGHT+STEP_GAP));

                bool step_on = target_pattern->query_note_on_for_step(i);
                const uint16_t colour = step_on ? 
                    (target_pattern->get_step_for_tick(ticks) == i ? RED : BLUE) :
                    (target_pattern->get_step_for_tick(ticks) == i ? RED : GREY);
                if (step_on) 
                    actual->fillRect(x, y, STEP_WIDTH, STEP_HEIGHT, colour);
                else
                    actual->drawRect(x, y, STEP_WIDTH, STEP_HEIGHT, colour);
            }

            base_row += (COLUMNS/STEP_HEIGHT) * (STEP_HEIGHT + STEP_GAP);
            tft->setCursor(0, base_row);

            /*for (int i = 0 ; i < target_pattern->steps ; i++) {
                tft->printf("%02i: ", i);
                tft->printf(get_note_name_c(behaviour_beatstep->sequence->get_pitch_at_step(i)));
                tft->println();
            }*/
            pos.y = tft->getCursorY();

            return pos.y; // + 40; //row + 12; //pos.y + (first_found - last_found); //127;  
        }
};