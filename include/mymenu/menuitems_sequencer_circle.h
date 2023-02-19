#include "sequencer/Sequencer.h"
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
            Serial.printf("CircleDisplay() setup_coordinates, tft width is %i\n", tft->width()); Serial.flush();
            //this->set_pattern(target_pattern);
            const size_t divisions = 16;
            const size_t degrees_per_iter = 360 / divisions;
            float size = 20.0*(tft->width()/2);
            int position = 4;
            for (int i = 0 ; i < divisions; i++) {
                Serial.printf("generating coordinate for position %i:\trad(cos()) is %f\n", i, radians(cos(i*degrees_per_iter*PI/180)));
                Serial.printf("generating coordinate for position %i:\trad(sin()) is %f\n", i, radians(sin(i*degrees_per_iter*PI/180)));
                coordinates_x[position] = (int)((float)size * radians(cos((i)*degrees_per_iter*PI/180)));
                coordinates_y[position] = (int)((float)size * radians(sin((i)*degrees_per_iter*PI/180)));
                Serial.printf("generating coordinate for position %i:\t[%i,%i]\n---\n", i, coordinates_x[i], coordinates_y[i]); Serial.flush();
                position++;
                position = position % divisions;
            }
            //this->set_sequencer(sequencer);
            Serial.println("CircleDisplay() finished setup_coordinates"); Serial.flush();
        }


        virtual int display(Coord pos, bool selected, bool opened) override {
            //return pos.y;
            pos.y = header(label, pos, selected, opened);

            tft->printf("ticks: %i step: %i\n", ticks, BPM_CURRENT_STEP_OF_BAR);

            if (this->target_sequencer==nullptr) {
                tft->println("No sequencer selected"); Serial.flush();
                return tft->getCursorY();
            }

            tft->setCursor(pos.x, pos.y);

            // we're going to use direct access to the underlying Adafruit library here
            #ifdef TFT_BODMER
                const DisplayTranslator_Bodmer *tft2 = (DisplayTranslator_Bodmer*)tft;
                TFT_eSPI *actual = tft2->tft;
            #else
                const DisplayTranslator_ST7789 *tft2 = (DisplayTranslator_ST7789*)tft;
                Adafruit_ST7789 *actual = tft2->tft;
            #endif

            for (int i = 0 ; i < 16 ; i++) {
                int16_t colour = BPM_CURRENT_STEP_OF_BAR == i ? RED : BLUE;
                actual->fillCircle(
                    (tft->width()/2) + coordinates_x[i], 
                    (tft->width()/3) + coordinates_y[i], 
                    3, 
                    colour
                );
            }

            return tft->height();
        }
};