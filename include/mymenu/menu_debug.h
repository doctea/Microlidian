#include "Config.h"
#include "menu.h"
#include "debug.h"
#include "menuitems_numbers.h"
#include "menuitems_listviewer.h"
#include "menuitems_lambda.h"
#include "cv_input.h"
#include "bpm.h"

#include "outputs/output_processor.h"
#include "sequencer/sequencing.h"

#ifdef ENABLE_CV_INPUT
    #include "ADS1X15.h"
#endif

#include "__version.h"

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
            tft->printf("Parameters: %i\n", parameter_manager->available_parameters->size());
            tft->printf("Free RAM: %u bytes\n", freeRam());
            tft->printf("Uptime: %02uh %02um %02us\n", time/60/60, (time/60)%60, (time)%60);
            tft->print("Serial: ");
            tft->print(Serial?"connected\n":"not connected\n");
            tft->print("USB host connected: ");
            /*tft->print(tud_connected()?"yes\n":"--\n");     // not expected to work: https://github.com/hathach/tinyusb/issues/2478#issuecomment-2094344075
            tft->print("USB host mounted: ");
            tft->print(tud_mounted()?"yes\n":"--\n");       // not expected to work: https://github.com/hathach/tinyusb/issues/2478#issuecomment-2094344075
            tft->print("USB host ready: ");*/
            tft->print(tud_ready()?"yes\n":"--\n");
            tft->print("Clock type: ");
            #if defined(USE_UCLOCK_GENERIC) && defined(USE_UCLOCK)
                tft->println("uClock - generic");
            #elif defined(USE_UCLOCK)
                tft->println("uClock - hardware");
            #else
                tft->println("Software");
            #endif
            tft->println("Built at " __BUILD_TIME__);
            tft->println("Git info: " COMMIT_INFO);
            #ifdef ENABLE_CV_INPUT
                tft->printf("ADS1X15 version: %s\n", (char*)ADS1X15_LIB_VERSION);
            #endif
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

    //ObjectNumberControl<Menu,int> *text_size_control = new ObjectNumberControl<Menu,int>("Text size", menu, &Menu::set_default_textsize, &Menu::get_default_textsize, nullptr, false);
    LambdaNumberControl<int8_t> *text_size_control = new LambdaNumberControl<int8_t>(
        "Text size", 
        [=](int8_t v) -> void { menu->set_default_textsize(v); }, 
        [=](void) -> int8_t { return menu->get_default_textsize(); } 
    );
    menu->add(text_size_control);

    ActionConfirmItem *reset_control = new ActionConfirmItem("RESET RP2040?", reset_rp2040);
    menu->add(reset_control);

    ActionConfirmItem *reset_bootloader = new ActionConfirmItem("FIRMWARE UPLOAD?", reset_upload_firmware);
    menu->add(reset_bootloader);

    SubMenuItemBar *bar = new SubMenuItemBar("Debug");

    LambdaToggleControl *debug_times_control = new LambdaToggleControl("Render times", [=](bool v) -> void { menu->setDebugTimes(v); }, [=]() -> bool { return menu->isDebugTimes(); }, nullptr);
    bar->add(debug_times_control);
    LambdaToggleControl *profiler_control = new LambdaToggleControl("Profiler", [=](bool v) -> void { menu->setProfileEnable(v); }, [=]() -> bool { return menu->isProfileEnable(); }, nullptr);
    bar->add(profiler_control);
    /*bar->add(debug_times_control);
    bar->add(new NumberControl<bool>("Extra", (bool*)&debug_flag, debug_flag, false, true));
    bar->add(new NumberControl<bool>("InSaNe", (bool*)&debug_stress_sequencer_load, debug_flag, false, true));
    menu->add(bar);*/
    //menu->add(debug_times_control);

    //bar->add(new NumberControl<int>("menu time", (int*)&menu_time, 0, 0, pow(2,31), (bool)false));
    //bar->add(new NumberControl<int>("tft time", (int*)&tft_time, 0, 0, pow(2,31), (bool)false));
    bar->add(new NumberControl<int>("missed micros", (int*)&missed_micros, 0, 0, (int32_t)pow(2,31), (bool)false));
    bar->add(new NumberControl<uint32_t>("ticks", (uint32_t*)&ticks, 0, 0, pow(2,31), (bool)false));
    menu->add(bar);

    menu->add(new DebugPanel());

    #ifdef ENABLE_CV_INPUT
        menu->add(new ToggleControl<bool>("CV Input", &cv_input_enabled, nullptr));
    #endif

    // basically just for debugging / testing speed
    LambdaToggleControl *enable_output_processor_control = new LambdaToggleControl("Enable Output Processor", [=](bool v) -> void { output_processor->set_enabled(v); }, [=](void) -> bool { return output_processor->is_enabled(); });
    menu->add(enable_output_processor_control);

    LambdaToggleControl *enable_sequencer_control = new LambdaToggleControl("Enable Sequencer", [=](bool v) -> void { sequencer->set_playing(v); }, [=](void) -> bool { return sequencer->is_running(); });
    menu->add(enable_sequencer_control);
    
    setup_messages_menu();
}