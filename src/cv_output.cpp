#ifdef ENABLE_CV_OUTPUT
    #include "cv_output.h"

    extern DAC8574 dac_output(ENABLE_CV_OUTPUT);

    bool cv_output_enabled = true;

    void setup_cv_output() {
        dac_output.begin();
    }
#endif