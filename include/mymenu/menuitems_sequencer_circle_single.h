#ifndef MENUITEMS_SEQUENCER_CIRCLE__H
#define MENUITEMS_SEQUENCER_CIRCLE__H

#include "sequencer/Euclidian.h"
#include "sequencer/Patterns.h"

#include "menuitems.h"

//#include "submenuitem_bar.h"
//#include "menuitems_object_selector.h"
//#define NUM_STEPS 16

#include <bpm.h>
#include <clock.h>

class SingleCircleDisplay : public MenuItem {
    public:
        //DeviceBehaviour_Beatstep *behaviour_beatstep = nullptr;
        //BaseSequencer *target_sequencer = nullptr;
        EuclidianPattern *target_pattern = nullptr;
        int_fast8_t coordinates_x[16];
        int_fast8_t coordinates_y[16];
        SingleCircleDisplay(const char *label, EuclidianPattern *target_pattern) : MenuItem(label, false) {
            this->set_target(target_pattern);
        }

        void on_add() override {
            MenuItem::on_add();
            setup_coordinates();
        }

        void set_target(EuclidianPattern *target_pattern) {
            this->target_pattern = target_pattern;
        }

        void setup_coordinates() {
            Debug_printf("SingleCircleDisplay() setup_coordinates, tft width is %i\n", tft->width()); Serial.flush();
            //this->set_pattern(target_pattern);
            const size_t divisions = 16;
            const float degrees_per_iter = 360.0 / divisions;
            float size = 20.0*(tft->width()/2);
            int position = 4;
            for (int i = 0 ; i < divisions; i++) {
                Debug_printf("generating coordinate for position %i:\trad(cos()) is %f\n", i, radians(cos(i*degrees_per_iter*PI/180)));
                Debug_printf("generating coordinate for position %i:\trad(sin()) is %f\n", i, radians(sin(i*degrees_per_iter*PI/180)));
                coordinates_x[position] = (int)((float)size * radians(cos(((float)i)*degrees_per_iter*PI/180.0)));
                coordinates_y[position] = (int)((float)size * radians(sin(((float)i)*degrees_per_iter*PI/180.0)));
                Debug_printf("generating coordinate for position %i:\t[%i,%i]\n---\n", i, coordinates_x[i], coordinates_y[i]); Serial.flush();
                position++;
                position = position % divisions;
            }
            //this->set_sequencer(sequencer);
            Debug_println("SingleCircleDisplay() finished setup_coordinates"); Serial.flush();
        }

        virtual int display(Coord pos, bool selected, bool opened) override {
            //return pos.y;
            int initial_y = pos.y;
            //pos.y = header(label, pos, selected, opened);
            //tft->printf("ticks:%4i step:%i\n", ticks, BPM_CURRENT_STEP_OF_BAR);
            
            /*static int last_rendered_step = -1;
            if (BPM_CURRENT_STEP_OF_BAR==last_rendered_step)
                return tft->height();
            last_rendered_step = BPM_CURRENT_STEP_OF_BAR;*/

            if (this->target_pattern==nullptr) {
                tft->println("No pattern selected"); Serial.flush();
                return tft->getCursorY();
            }

            tft->setCursor(pos.x, pos.y);

            static const int_fast8_t circle_center_x = tft->width()/4;
            static const int_fast8_t circle_center_y = pos.y + ((tft->height() - pos.y) / 2);

            // draw circle
            int_fast8_t first_x, first_y;
            int_fast8_t last_x, last_y;
            int_fast8_t count = 0;
            //BasePattern *pattern = target_sequencer->get_pattern(seq);
            //int16_t colour = color565(255 * seq, 255 - (255 * seq), seq) + (seq*8);
            uint_fast16_t pattern_colour = target_pattern->get_colour();
            bool is_step_on = target_pattern->query_note_on_for_step(BPM_CURRENT_STEP_OF_PHRASE);
            if (!is_step_on) pattern_colour = tft->halfbright_565(pattern_colour);
            // todo: if STEPS_PER_PHRASE is a multiple of get_steps, should be able to limit number of loops we do here?
            for (int i = 0 ; i < STEPS_PER_PHRASE/*max(target_pattern->get_steps(),16*/ ; i++) {
                int_fast8_t coord_x = circle_center_x + coordinates_x[i%16];
                int_fast8_t coord_y = circle_center_y + coordinates_y[i%16];
                if (target_pattern->query_note_on_for_step(i)) {
                    if (count>0) {
                        tft->drawLine(
                            last_x, last_y, coord_x, coord_y,
                            pattern_colour
                        );
                    } else {
                        first_x = coord_x;
                        first_y = coord_y;
                    }
                    last_x = circle_center_x + coordinates_x[i%16];
                    last_y = circle_center_y + coordinates_y[i%16];

                    tft->fillCircle(last_x, last_y, 6, (is_step_on ? target_pattern->get_colour() : pattern_colour));  // draw a slightly larger colour circle
                    count++;
                }
            }
            if (count>1) {
                tft->drawLine(last_x, last_y, first_x, first_y, pattern_colour);
            }

            // draw step markers around circle
            const int_fast8_t radius = 2;
            for (int_fast8_t i = 0 ; i < 16 ; i++) {
                const int_fast16_t colour = BPM_CURRENT_STEP_OF_BAR == i ? RED : (is_step_on ? target_pattern->get_colour() : pattern_colour);
                tft->fillCircle(
                    circle_center_x + coordinates_x[i], 
                    circle_center_y + coordinates_y[i], 
                    radius, 
                    colour
                );
            }

            return tft->height();
        }
};

#endif