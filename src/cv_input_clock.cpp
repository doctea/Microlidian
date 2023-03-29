#include "Config.h"

#include "cv_input_clock.h"
#include <clock.h>

#include <ParameterManager.h>

int time_between_cv_input_updates = TIME_BETWEEN_CV_INPUT_UPDATES;

void clock_mode_changed(ClockMode old_mode, ClockMode new_mode) {
    if (new_mode==CLOCK_EXTERNAL_CV) {
        time_between_cv_input_updates = 2;  // increase speed of reads from ADC so that we don't miss clock ticks
        ticks = 0; // so that the next tick we receive is treated as 0?
        playing = true;
    } else {
        // reset time between ADC reads to the default
        time_between_cv_input_updates = TIME_BETWEEN_CV_INPUT_UPDATES;  
    }
}

bool actual_check_cv_clock_ticked() {
    static uint32_t last_checked_time;
    static bool already_high = false;
    //Serial.println("actual_check_cv_clock_ticked");
    
    //parameter_manager->voltage_sources->get(0)->update();
    float v = parameter_manager->voltage_sources->get(0)->get_voltage();
    //Serial.printf("\tgot value %3.3f\n", parameter_manager->voltage_sources->get(0)->get_voltage()); 
    if (!already_high && v >= 3.0) {
        already_high = true;
        Serial.printf("%u: rise, got value %3.3f, ticking!\n", millis(), v);
        last_checked_time = millis();
        return true;
    } else if (already_high && v <= 0.5) {
        Serial.printf("%u: fall!\n", millis());
        already_high = false;
    } else {
        //Serial.printf("%u: neither rose nor fell? value is %3.3f!\n", millis(), v);
    }
    last_checked_time = millis();

    return false;
}
