#include "Config.h"
#include "menu.h"
#include "debug.h"
//#include "mymenu/menu_usb.h"
//#include "mymenu/menu_behaviours.h"
#include "menuitems_numbers.h"
//#include "submenuitem_bar.h"
#include "menuitems_listviewer.h"

//#include "menuitems_selector.h"

#include "cv_input.h"

#include "bpm.h"

//#include "__version.h"

extern bool debug_flag, debug_stress_sequencer_load;

class DebugPanel : public MenuItem {
    public:
        DebugPanel() : MenuItem("Debug") {
            this->selectable = false;
        }

        virtual int display(Coord pos, bool selected, bool opened) override {
            unsigned long time = millis()/1000;
            tft->setCursor(pos.x,pos.y);
            header("Statistics:", pos, selected, opened);
            tft->println("Built at " __TIME__ " on " __DATE__);
            //tft->println("Git info: " COMMIT_INFO);
            //tft->printf("Free RAM: %u bytes\n", freeRam());
            tft->printf("Uptime: %02uh %02um %02us\n", time/60/60, (time/60)%60, (time)%60);
            tft->print("Serial: ");
            tft->print(Serial?"connected\n":"not connected\n");
            return tft->getCursorY();
        }
};


extern volatile uint32_t ticks;

//float bpm_selector_values[] = { 60, 90, 120, 150, 180, 500, 1000, 2000, 3000 };

#ifndef GDB_DEBUG
FLASHMEM // void setup_debug_menu() causes a section type conflict with void Menu::start()
#endif
void setup_debug_menu() {
    /*menu->add_page("Behaviours/USB");

    #ifdef ENABLE_USB
        USBDevicesPanel *usbdevices_panel = new USBDevicesPanel();
        menu->add(usbdevices_panel);
    #endif

    BehavioursPanel *behaviours_panel = new BehavioursPanel();
    menu->add(behaviours_panel);

    ////*/

    menu->add_page("Debug");

    //ActionConfirmItem *reset_control = new ActionConfirmItem("RESET TEENSY?", reset_teensy);
    //menu->add(reset_control);

    /*SelectorControl<float> *bpm_selector = new SelectorControl<float>("BPM");
    bpm_selector->available_values = bpm_selector_values;
    bpm_selector->num_values = sizeof(bpm_selector_values)/sizeof(float);
    bpm_selector->setter = set_bpm;
    bpm_selector->getter = get_bpm;
    menu->add(bpm_selector);*/

    SubMenuItemBar *bar = new SubMenuItemBar("Debug");

    ObjectToggleControl<Menu> *debug_times_control = new ObjectToggleControl<Menu>("Render times", menu, &Menu::setDebugTimes, &Menu::isDebugTimes, nullptr);
    /*bar->add(debug_times_control);
    bar->add(new NumberControl<bool>("Extra", (bool*)&debug_flag, debug_flag, false, true));
    bar->add(new NumberControl<bool>("InSaNe", (bool*)&debug_stress_sequencer_load, debug_flag, false, true));
    menu->add(bar);*/
    menu->add(debug_times_control);

    bar->add(new NumberControl<int>("menu time", (int*)&menu_time, 0, 0, pow(2,31), (bool)false));
    bar->add(new NumberControl<int>("tft time", (int*)&tft_time, 0, 0, pow(2,31), (bool)false));
    bar->add(new NumberControl<int>("missed micros", (int*)&missed_micros, 0, 0, pow(2,31), (bool)false));
    bar->add(new NumberControl<uint32_t>("ticks", (uint32_t*)&ticks, 0, 0, pow(2,31), (bool)false));
    menu->add(bar);

    menu->add(new DebugPanel());

    #ifdef ENABLE_CV_INPUT
        menu->add(new ToggleControl<bool>("CV Input", &cv_input_enabled, nullptr));
    #endif
    
    menu->add(new ListViewerMenuItem("Message history", messages_log));
}