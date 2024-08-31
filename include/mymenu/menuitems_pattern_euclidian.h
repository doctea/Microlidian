#define ENABLE_OUTPUT_SELECTOR      // euclidian output selector control that is causing us so many crashy problems
#define ENABLE_STEP_DISPLAYS
#define ENABLE_OTHER_CONTROLS

#include "submenuitem_bar.h"

#include "mymenu/menuitems_sequencer.h"
#include "mymenu/menuitems_sequencer_circle_single.h"

#include "mymenu/menuitems_outputselectorcontrol.h"

#include "outputs/output_processor.h"
extern MIDIOutputProcessor *output_processor;

// compound control for Euclidian Patterns; shows step sequence view, animated circle sequence view, and controls 
class EuclidianPatternControl : public SubMenuItemBar {
    public:

    #ifdef ENABLE_STEP_DISPLAYS
        SingleCircleDisplay *circle_display = nullptr;
        PatternDisplay *step_display = nullptr;
    #endif
    EuclidianPattern *pattern = nullptr;

    EuclidianPatternControl(const char *label, EuclidianPattern *pattern) : SubMenuItemBar(label), pattern(pattern) {
        #ifdef ENABLE_STEP_DISPLAYS
            this->circle_display = new SingleCircleDisplay(label, pattern);     // circle display first - don't add this as a submenu item, because it isn't selectable
            this->step_display = new PatternDisplay(label, pattern, false, false);    // step sequence view next - not selectable, don't draw header
        #endif

        #ifdef ENABLE_OUTPUT_SELECTOR
            OutputSelectorControl<EuclidianPattern> *selector = new OutputSelectorControl<EuclidianPattern>(
                "Output",
                pattern,
                &EuclidianPattern::set_output,
                &EuclidianPattern::get_output,
                output_processor->nodes,
                pattern->output
            );
            selector->go_back_on_select = true;
            this->add(selector);
        #endif

        #ifdef ENABLE_OTHER_CONTROLS
            //SubMenuItemBar *bar = new SubMenuItemBar("Arguments");
            //Menu *bar = menu;
            this->add(new ObjectNumberControl<EuclidianPattern,int8_t> ("Steps",    pattern, &EuclidianPattern::set_steps,      &EuclidianPattern::get_steps,    nullptr, 1, pattern->maximum_steps, true, true));
            this->add(new ObjectNumberControl<EuclidianPattern,int8_t> ("Pulses",   pattern, &EuclidianPattern::set_pulses,     &EuclidianPattern::get_pulses,   nullptr, 1, STEPS_PER_BAR, true, true));
            this->add(new ObjectNumberControl<EuclidianPattern,int8_t> ("Rotation", pattern, &EuclidianPattern::set_rotation,   &EuclidianPattern::get_rotation, nullptr, 1, pattern->maximum_steps, true, true));
            this->add(new ObjectNumberControl<EuclidianPattern,int8_t> ("Duration", pattern, &EuclidianPattern::set_duration,   &EuclidianPattern::get_duration, nullptr, MINIMUM_DURATION, PPQN*BEATS_PER_BAR, true, true));  // minimum duration needs to be 2 , otherwise can end up with note on's getting missed by usb_teensy_clocker!
            //menu->debug = true;
            this->add(new ObjectToggleControl<EuclidianPattern> ("Locked", pattern, &EuclidianPattern::set_locked, &EuclidianPattern::is_locked));
            this->add(new ObjectActionConfirmItem<EuclidianPattern> ("Store" /*"Store as default"*/, pattern, &EuclidianPattern::store_current_arguments_as_default));
        #endif
    }

    virtual void on_add() override {
        SubMenuItemBar::on_add();
        #ifdef ENABLE_STEP_DISPLAYS
            if (this->circle_display!=nullptr) {
                this->circle_display->set_tft(this->tft);
                this->circle_display->on_add();
            }
            if (this->step_display!=nullptr) {
                this->step_display->set_tft(this->tft);
                this->step_display->on_add();
            }
        #endif
    }


    virtual int display(Coord pos, bool selected, bool opened) override {
        //pos.y = header(label, pos, selected, opened);
        tft->setCursor(pos.x, pos.y);
        colours(opened, opened ? GREEN : this->default_fg, this->default_bg);

        #ifdef ENABLE_STEP_DISPLAYS
            if (this->step_display!=nullptr)
                pos.y = this->step_display->display(pos, selected, opened); // display the step sequencer across the top
        #endif

        uint_fast16_t start_y = pos.y;        // y to start drawing at (just under header)
        uint_fast16_t finish_y = pos.y;       // highest y that we finished drawing at

        // draw all the sub-widgets
        //int width_per_item = this->tft->width() / (this->items->size() /*+1*/);
        int start_x = tft->width()/2;
        Debug_printf(F("display in SubMenuItemBar got width_per_item=%i\tfrom tftwidth\t%i / itemsize\t%i\n"), width_per_item, this->tft->width(), this->items->size());
        const uint_fast16_t items_size = this->items->size();
        for (uint_fast16_t item_index = 0 ; item_index < items_size ; item_index++) {

            uint_fast16_t column = (item_index-1)%2==1;   // first menu item ('output' should span both columns, s
            if (item_index==0   // first item forced to first column
                //|| item_index==items_size-1    // last item forced to first column?
            )
            column = 0;

            if (column==0)
                start_x = tft->width()/2;
            else 
                start_x = (tft->width()/2)  + (tft->width()/4);

            bool wrap = item_index==0   // first item moves cursor to next row
                        || column==1    // second column moves cursor to next row
                        //|| item_index==items_size-2
                    ;    // last item moves cursor to next row

            //int width = this->get_max_pixel_width(item_index);
            uint_fast16_t width = ((wrap && column==0) ? this->tft->width()/2 : this->tft->width()/4); ///this->tft->characterWidth();
            //if (item_index==items_size-1) width = width/2;

            //if (item_index==0) width -= tft->characterWidth();  // adjust by 1 character if necessary
            const int temp_y = this->small_display(
                item_index, 
                start_x, 
                start_y, 
                width, //width_per_item, 
                this->currently_selected==(int)item_index, 
                this->currently_opened==(int)item_index,
                !opened && selected
            );


            //start_x += width;
            //Serial.printf("for item %i '%s':\tgot column=\t%i, wrap=\t%s => start_x=%i\t, start_y=%i\t((item_index-1)%%2=%i\n", item_index, this->items->get(item_index)->label, column, wrap?"Y":"N", start_x, start_y, (item_index-1)%2);
            //if (item_index==0 || (item_index-1)%2==0 || item_index==this->items->size()-2)
            if (wrap)
                start_y = temp_y;
            if (temp_y>finish_y)
                finish_y = temp_y;
        }

        #ifdef ENABLE_STEP_DISPLAYS
            if (this->circle_display!=nullptr)
                this->circle_display->display(pos, selected, opened);
        #endif

        return finish_y;
    }

    virtual inline int get_max_pixel_width(uint_fast16_t item_number) {
        if (item_number==0 || item_number==this->items->size()-1)
            return this->tft->width() / 2;
        else
            return this->tft->width() / 4;
    }
};