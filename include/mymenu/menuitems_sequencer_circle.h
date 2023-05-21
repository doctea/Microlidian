#ifndef MENUITEMS_SEQUENCER_CIRCLE__H
#define MENUITEMS_SEQUENCER_CIRCLE__H

#ifdef ENABLE_SCREEN

#include "sequencer/Sequencer.h"
#include "sequencer/Patterns.h"

#include "menuitems.h"

//#include "submenuitem_bar.h"
//#include "menuitems_object_selector.h"
//#define NUM_STEPS 16

#include <bpm.h>
#include <clock.h>

class CircleDisplay : public MenuItem {
    public:
        //DeviceBehaviour_Beatstep *behaviour_beatstep = nullptr;
        BaseSequencer *target_sequencer = nullptr;
        int coordinates_x[16];
        int coordinates_y[16];
        CircleDisplay(const char *label, BaseSequencer *sequencer) : MenuItem(label) {
            this->set_sequencer(sequencer);
        }

        void on_add() override {
            MenuItem::on_add();
            setup_coordinates();
        }

        void set_sequencer(BaseSequencer *sequencer) {
            this->target_sequencer = sequencer;
        }

        void setup_coordinates() {
            Debug_printf("CircleDisplay() setup_coordinates, tft width is %i\n", tft->width()); Serial.flush();
            //this->set_pattern(target_pattern);
            const size_t divisions = 16;
            const size_t degrees_per_iter = 360 / divisions;
            float size = 20.0*(tft->width()/2);
            int position = 4;
            for (int i = 0 ; i < divisions; i++) {
                Debug_printf("generating coordinate for position %i:\trad(cos()) is %f\n", i, radians(cos(i*degrees_per_iter*PI/180)));
                Debug_printf("generating coordinate for position %i:\trad(sin()) is %f\n", i, radians(sin(i*degrees_per_iter*PI/180)));
                coordinates_x[position] = (int)((float)size * radians(cos((i)*degrees_per_iter*PI/180)));
                coordinates_y[position] = (int)((float)size * radians(sin((i)*degrees_per_iter*PI/180)));
                Debug_printf("generating coordinate for position %i:\t[%i,%i]\n---\n", i, coordinates_x[i], coordinates_y[i]); Serial.flush();
                position++;
                position = position % divisions;
            }
            //this->set_sequencer(sequencer);
            Debug_println("CircleDisplay() finished setup_coordinates"); Serial.flush();
        }

        virtual int display(Coord pos, bool selected, bool opened) override {
            //return pos.y;
            int initial_y = pos.y;
            //pos.y = header(label, pos, selected, opened);

            tft->printf("ticks:%4i step:%i\n", ticks, BPM_CURRENT_STEP_OF_BAR);
            
            /*static int last_rendered_step = -1;
            if (BPM_CURRENT_STEP_OF_BAR==last_rendered_step)
                return tft->height();
            last_rendered_step = BPM_CURRENT_STEP_OF_BAR;*/

            if (this->target_sequencer==nullptr) {
                tft->println("No sequencer selected"); Serial.flush();
                return tft->getCursorY();
            }

            tft->setCursor(pos.x, pos.y);

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

            static const int_fast8_t circle_center_x = tft->width()/4;
            static const int_fast8_t circle_center_y = tft->width()/3;

            // draw circle
            for (int_fast8_t seq = 0 ; seq < target_sequencer->number_patterns ; seq++) {
                int_fast8_t first_x, first_y;
                int_fast8_t last_x, last_y;
                int_fast8_t count = 0;
                BasePattern *pattern = target_sequencer->get_pattern(seq);
                //int16_t colour = color565(255 * seq, 255 - (255 * seq), seq) + (seq*8);
                uint16_t colour = pattern->colour;
                if (!pattern->query_note_on_for_step(BPM_CURRENT_STEP_OF_BAR))
                    colour = tft->halfbright_565(colour);
                for (int i = 0 ; i < 16 ; i++) {
                    int_fast8_t coord_x = circle_center_x + coordinates_x[i];
                    int_fast8_t coord_y = circle_center_y + coordinates_y[i];
                    if (pattern->query_note_on_for_step(i)) {
                        if (count>0) {
                            actual->drawLine(
                                last_x, last_y, coord_x, coord_y,
                                colour
                            );
                        } else {
                            first_x = coord_x;
                            first_y = coord_y;
                        }
                        last_x = circle_center_x + coordinates_x[i];
                        last_y = circle_center_y + coordinates_y[i];
                        count++;
                    }
                }
                if (count>1) {
                    actual->drawLine(last_x, last_y, first_x, first_y, colour);
                }
            }

            // draw step markers around circle
            const int_fast8_t radius = 2;
            for (int_fast8_t i = 0 ; i < 16 ; i++) {
                int16_t colour = BPM_CURRENT_STEP_OF_BAR == i ? RED : BLUE;
                actual->fillCircle(
                    circle_center_x + coordinates_x[i], 
                    circle_center_y + coordinates_y[i], 
                    radius, 
                    colour
                );
            }

            // draw flashy blocks for every patterns
            tft->setCursor(tft->width()/2, ++initial_y);
            colours(false, C_WHITE);
            tft->println(" St Pu Ro   St Pu Ro");
            tft->setCursor(tft->width()/2, pos.y);
            for (int_fast8_t seq = 0 ; seq < target_sequencer->number_patterns ; seq++) {
                int_fast8_t column = seq / 10;
                int_fast8_t row = 1+(seq % 10);
                tft->setCursor((tft->width()/2) + (column*65), initial_y + (row*8)); //tft->getCursorY());
                //tft->setCursor((tft->width()/2) + (seq/10), tft->getCursorY());
                BasePattern *pattern = target_sequencer->get_pattern(seq);
                colours(pattern->query_note_on_for_step(BPM_CURRENT_STEP_OF_BAR), pattern->colour, BLACK);
                tft->print(" ");
                colours(false, C_WHITE);
                //tft->printf("%i %s\n", seq, pattern->get_summary());
                tft->print(pattern->get_summary());
            }

            return tft->height();
        }
};

#endif

#endif