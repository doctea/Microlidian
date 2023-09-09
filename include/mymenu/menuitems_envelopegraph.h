#ifndef MENU_ENVELOPEGRAPH_VIEW_MENUITEMS__INCLUDED
#define MENU_ENVELOPEGRAPH_VIEW_MENUITEMS__INCLUDED

#include "Arduino.h"

#include "menu.h"
#include "colours.h"

#include "../outputs/envelopes.h"

#include <LinkedList.h>

#ifndef PARAMETER_INPUT_GRAPH_HEIGHT
    #define PARAMETER_INPUT_GRAPH_HEIGHT 50
#endif

#include "bpm.h"    // because we need to know the current ticks

//template<unsigned long memory_size>
class EnvelopeDisplay : public MenuItem
{
    public:
        EnvelopeOutput *envelope = nullptr;

        // todo: remember an int type instead of a float, for faster drawing
        typedef float memory_log;
        unsigned long memory_size = 240;
        memory_log *logged = nullptr;

        //EnvelopeDisplay(char *label, unsigned long memory_size, EnvelopeOutput *envelope) : MenuItem(label) {
        EnvelopeDisplay(const char *label, EnvelopeOutput *envelope) : MenuItem(label) {
            this->envelope = envelope;
            this->memory_size = memory_size;

            this->selectable = false;

            //if (parameter_input!=nullptr) 
            //    this->set_default_colours(parameter_input->colour);
        }

        virtual void configure(EnvelopeOutput *envelope) {
            this->envelope = envelope;
        }

        unsigned long ticks_to_memory_step(uint32_t ticks) {
            return ( ticks % memory_size );
        }

        virtual int display(Coord pos, bool selected, bool opened) override {
            //Serial.println("MidiOutputSelectorControl display()!");
            tft->setTextSize(0);

            /*#define DISPLAY_INFO_IN_LABEL
            #ifdef DISPLAY_INFO_IN_LABEL
                static char custom_label[MAX_LABEL_LENGTH*2];

                snprintf(custom_label, MAX_LABEL_LENGTH*2, "[%s] %-3s >%-3i %-3s>",
                    //label,                    
                    this->parameter_input!=nullptr ? (char*)this->parameter_input->getInputInfo()  : "",
                    this->parameter_input!=nullptr ? (char*)this->parameter_input->getInputValue() : "",
                    //(int)(this->logged[(ticks%LOOP_LENGTH_TICKS] * 100.0)
                    this->parameter_input!=nullptr ? (int)(this->parameter_input->get_normal_value()*100.0) : 0, 
                    (char*)this->parameter_input->getOutputValue()
                );
                colours(selected, parameter_input->colour, BLACK);
                pos.y = header(custom_label, pos, selected, opened);         
            #else 
                pos.y = header(label, pos, selected, opened);                      
                //tft->setCursor(pos.x, pos.y);
            #endif*/
            pos.y = tft->getCursorY();

            // switch back to colour-on-black for actual display
            //colours(false, parameter_input->colour, BLACK);

            const uint16_t base_row = pos.y;
            //static float ticks_per_pixel = (float)memory_size / (float)tft->width();
            static float tickers_per_pixel = 1;

            int stage_colours[] = {
                0x8080,
                GREEN,
                YELLOW,
                ORANGE,
                RED,
                PURPLE,
                BLUE
            };

            int last_y = 0;
            for (int screen_x = 0 ; screen_x < tft->width() ; screen_x++) {
                //const uint16_t tick_for_screen_X = ticks_to_memory_step((int)((float)screen_x * ticks_per_pixel)); // the tick corresponding to this screen position
                const uint16_t tick_for_screen_X = screen_x;
                float value = (float)envelope->graph[tick_for_screen_X].value;
                byte stage = envelope->graph[tick_for_screen_X].stage;
                const int y = PARAMETER_INPUT_GRAPH_HEIGHT - ((value/127.0) * PARAMETER_INPUT_GRAPH_HEIGHT);
                if (screen_x != 0) {
                    //int last_y = GRAPH_HEIGHT - (this->logged[tick_for_screen_X] * GRAPH_HEIGHT);
                    //actual->drawLine(screen_x-1, base_row + last_y, screen_x, base_row + y, YELLOW);                    
                    tft->drawLine(screen_x-1, base_row + last_y, screen_x, base_row + y, stage_colours[stage]); //parameter_input->colour);                    
                    if (envelope->invert)
                        tft->drawLine(screen_x, base_row,     screen_x, base_row + y, stage_colours[stage]);
                    else
                        tft->drawLine(screen_x, base_row + y, screen_x, base_row + PARAMETER_INPUT_GRAPH_HEIGHT, stage_colours[stage]);
                }
                //actual->drawFastHLine(screen_x, base_row + y, 1, GREEN);
                last_y = y;
            }

            tft->setCursor(pos.x, pos.y + PARAMETER_INPUT_GRAPH_HEIGHT + 5);    // set cursor to below the graph's output

            //if (this->parameter_input!=nullptr && this->parameter_input->hasExtra())
            //    tft->printf((char*)"Extra: %s\n", (char*)this->parameter_input->getExtra());

            return tft->getCursorY();
        }
};

class EnvelopeIndicator : public MenuItem {
    public:
    EnvelopeOutput *envelope = nullptr;

    const char *stage_labels[6] = {
        "Off",
        "Attack",
        "Hold",
        "Decay",
        "Sustain",
        "Release"
    };

    EnvelopeIndicator(const char *label, EnvelopeOutput *envelope) : MenuItem(label) {
        this->envelope = envelope;
    }

    virtual int display(Coord pos, bool selected, bool opened) override {
        tft->printf(" Name: %s    ", envelope->label);
        tft->printf("   CC: %i\n", envelope->midi_cc);
        tft->printf("Stage: %7s   ", (char*)stage_labels[envelope->last_state.stage]);
        tft->printf("Level: %i\n", envelope->last_state.lvl_now);
        
        return tft->getCursorY();
    }
};

#endif
