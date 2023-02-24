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
        PatternDisplay(const char *label, SimplePattern *target_pattern) : MenuItem(label) {
            this->set_pattern(target_pattern);
        }

        void set_pattern(SimplePattern *pattern) {
            this->target_pattern = pattern;
        }

        virtual int display(Coord pos, bool selected, bool opened) override {
            char label_info[MENU_C_MAX];
            snprintf(label_info, MENU_C_MAX, "%s: Steps=%2i Pulses=%2i Rot=%2i", this->label, 
                ((EuclidianPattern*)target_pattern)->arguments.steps, 
                ((EuclidianPattern*)target_pattern)->arguments.pulses, 
                ((EuclidianPattern*)target_pattern)->arguments.rotation
            );
            pos.y = header(label_info, pos, selected, opened);

            if (this->target_pattern==nullptr) {
                tft->println("No pattern selected");
                return tft->getCursorY();
            }

            tft->setCursor(pos.x, pos.y);

            uint16_t base_row = pos.y;
            //static float ticks_per_pixel = (float)LOOP_LENGTH_TICKS / (float)tft->width();

            // we're going to use direct access to the underlying library here
            #ifdef TFT_BODMER
                static const DisplayTranslator_Bodmer *tft2 = (DisplayTranslator_Bodmer*)tft;
                #ifdef BODMER_SPRITE
                    static TFT_eSprite *actual = tft2->tft;
                #else
                    static TFT_eSPI *actual = tft2->tft;
                #endif
            #else
                static const DisplayTranslator_ST7789 *tft2 = (DisplayTranslator_ST7789*)tft;
                static Adafruit_ST7789 *actual = tft2->tft;
            #endif

            /*#define STEP_WIDTH 8
            #define STEP_HEIGHT 8
            #define STEP_GAP 2*/
            #define MAX_COLUMNS 16
            //int columns = target_pattern->get_steps();

            static int width_per_cell = tft->width() / MAX_COLUMNS;
            static int STEP_GAP = 2; //width_per_cell - (width_per_cell/4);
            static int STEP_WIDTH = width_per_cell - STEP_GAP;
            static int STEP_HEIGHT = STEP_WIDTH;

            for (int i = 0 ; i < target_pattern->get_steps() ; i++) {
                int row = i / MAX_COLUMNS;
                int col = i % MAX_COLUMNS;

                int x = col * (STEP_WIDTH+STEP_GAP);
                int y = base_row + (row*(STEP_HEIGHT+STEP_GAP));

                int step_for_tick = target_pattern->get_step_for_tick(ticks);

                bool step_on = target_pattern->query_note_on_for_step(i);
                const uint16_t colour = step_on ? 
                    (step_for_tick == i ? RED : BLUE) :     // current step active
                    (step_for_tick == i ? RED : GREY);      // current step inactive
                if (step_on) 
                    actual->fillRect(x, y, STEP_WIDTH, STEP_HEIGHT, colour);
                else {
                    actual->drawRect(x, y, STEP_WIDTH, STEP_HEIGHT, colour);
                    actual->fillRect(x + 1, y + 1, STEP_WIDTH-2, STEP_HEIGHT-2, BLACK);
                }
            }

            base_row += (MAX_COLUMNS/STEP_HEIGHT) * ((STEP_HEIGHT + STEP_GAP) * (target_pattern->get_steps() / MAX_COLUMNS));
            //base_row *= (target_pattern->get_steps() / MAX_COLUMNS);
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