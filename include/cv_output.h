#pragma once

#ifdef ENABLE_CV_OUTPUT
    #include "DAC8574.h"

    extern DAC8574 *dac_output;

    //extern bool calibrating;
    extern bool cv_output_enabled;

    void setup_cv_output();

    /*float get_calibrated_voltage(float intended_voltage);   
    void calibrate_unipolar_minimum(int, char*);
    void calibrate_unipolar_maximum(int, char*);*/

    #include "parameters/calibration.h"

#endif