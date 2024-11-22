#pragma once

#ifdef ENABLE_CV_OUTPUT
    #include "DAC8574.h"

    extern DAC8574 dac_output;

    extern bool calibrating;

    void setup_cv_output();
    float get_calibrated_voltage(float intended_voltage);

    extern bool cv_output_enabled;

#endif