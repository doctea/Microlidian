#pragma once

#ifdef ENABLE_CV_OUTPUT
    #include "DAC8574.h"

    extern DAC8574 *dac_output;

    extern bool calibrating;

    void setup_cv_output();
    float get_calibrated_voltage(float intended_voltage);

    extern bool cv_output_enabled;

    void calibrate_unipolar_minimum(int, char*);
    void calibrate_unipolar_maximum(int, char*);

    class VoltageParameterInput;
    uint16_t calibrate_find_dac_value_for(int channel, VoltageParameterInput *src, float intended_voltage, bool inverted);
    uint16_t calibrate_find_dac_value_for(int channel, char *input_name, float intended_voltage, bool inverted);

#endif