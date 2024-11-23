#ifdef ENABLE_CV_OUTPUT
    #include "cv_output.h"

    #include "menu.h"
    #include "menuitems_action.h"

    #include "cv_input.h"

    #include "SimplyAtomic.h"
    #include "core_safe.h"

    DAC8574 dac_output(ENABLE_CV_OUTPUT);

    bool cv_output_enabled = true;

    bool calibrating = false;

    void start_calibration() {
        calibrating = true;
    }

    volatile float uni_max_output_voltage = 10.30, uni_min_output_voltage = -0.33;
    volatile float bi_max_output_voltage = uni_max_output_voltage,  bi_min_output_voltage = uni_min_output_voltage;

    #define O_VALUE  1024
    #define FS_VALUE 64512

    int32_t OE = 0;  // OE = measured DAC error at code 1024 (in LSBs)
    int32_t FSE = 0; // FSE = measured DAC error at code 64512 (in LSBs)

    /*
    from dac8574 datasheet at: https://www.ti.com/lit/ds/symlink/dac8574.pdf
    DIN = DDIN – OE – (FSE – OE) × (DDIN – 1024) ÷ 64512
    where:
    DIN = Digital input code to the DAC after offset and gain correction
    DDIN = Digital input code to the DAC before offset and gain correction
    OE = measured DAC error at code 1024 (in LSBs)
    FSE = measured DAC error at code 64512 (in LSBs)
    */

    void calibrate_unipolar_maximum() {
        VoltageParameterInput *src = (VoltageParameterInput*)parameter_manager->getInputForName("A");

        //acquire_lock();
        calibrating = true;
        ATOMIC(){
            // determine voltage at 0 point
            dac_output.write(0, FS_VALUE);
            delay(100);
            parameter_manager->update_voltage_sources();
            parameter_manager->update_inputs();
            src->read();
            parameter_manager->update_voltage_sources();
            parameter_manager->update_inputs();
            src->read();
            uni_min_output_voltage = src->get_voltage();
            uni_min_output_voltage = src->get_voltage();
            
            float v = src->fetch_current_voltage();
            Serial.printf("  FS_VALUE Target=%3.3f, ", FS_VALUE / 65535.0);
            v = 10.0 - v;
            Serial.printf("calibrate_unipolar_maximum got v = %3.3f", v);
            v = v / 10.0;
            FSE = FS_VALUE - (v * 65535.0);
            Serial.printf(" => FSE=%i\n", FSE);
            //FSE *= -1;

            messages_log_add(String("uni: output  ")+String(FS_VALUE)+" => "+ String(FSE));
        }
        //calibrating = false;
        //release_lock();
    }

    void calibrate_unipolar_minimum() {
        VoltageParameterInput *src = (VoltageParameterInput*)parameter_manager->getInputForName("A");

        //acquire_lock();
        calibrating = true;
        ATOMIC(){
            dac_output.write(0, O_VALUE);
            delay(100);
            parameter_manager->update_voltage_sources();
            parameter_manager->update_inputs();
            src->read();
            uni_max_output_voltage = src->get_voltage();
            parameter_manager->update_voltage_sources();
            parameter_manager->update_inputs();
            src->read();
            uni_max_output_voltage = src->get_voltage();
            parameter_manager->update_voltage_sources();
            parameter_manager->update_inputs();
            src->read();
            uni_max_output_voltage = src->get_voltage();

            float v = src->fetch_current_voltage();
            Serial.printf("  O_VALUE Target=%3.3f, ", O_VALUE / 65535.0);
            v = 10.0 - v;
            Serial.printf("calibrate_unipolar_minimum got v = %3.3f", v);
            v = v / 10.0;
            OE = O_VALUE - (v * 65535.0);
            Serial.printf(" => OE=%i\n", OE);
            OE *= -1;

            messages_log_add(String("uni: output ")+String(O_VALUE)+ " => " + String(OE));
        }
        //calibrating = false;
        //release_lock();
    }


    /*void calibrate_bipolar() {
        acquire_lock();
        ATOMIC(){
            // determine voltage at 0 point
            VoltageParameterInput *src = (VoltageParameterInput*)parameter_manager->getInputForName("A");

            dac_output.write(0, 0);
            delay(500);
            parameter_manager->update_inputs();
            bi_min_output_voltage = src->get_voltage();

            delay(500);

            dac_output.write(0, 65535);
            delay(500);
            parameter_manager->update_inputs();
            bi_max_output_voltage = src->get_voltage();

            messages_log_add(String(" bi: output     0 => ") + String(bi_min_output_voltage));
            messages_log_add(String(" bi: output "65535" => ") + String(bi_max_output_voltage));
        }
        release_lock();
    }*/

    float map(float x, float in_min, float in_max, float out_min, float out_max) {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    // todo: returns intended voltage calibrated for the hardware...
    /*float get_calibrated_voltage(float intended_voltage) {
        // if 0.0 gives -0.351
            // then to get real 0.0 we need to add 0.351 to it
        // if 10.0 gives 10.297
            // then to get real 10.0 we need to subtract 0.297 from it
            // so therefore max should be 10.0 - (10.297-10.0) ?

        // zero offset = uni_min_output_voltage * -65535
        float zero_offset = uni_min_output_voltage *- 1; // * -65535.0;
        float apex_offset = uni_max_output_voltage - 10.0;
        //float total_range = uni_max_output_voltage - uni_min_output_voltage;
        //float total_range = uni_max_output_voltage - zero_offset + apex_offset;

        //intended_voltage += zero_offset;
        //intended_voltage /= 10.0;
        //intended_voltage *= (total_range/10.0);
        //intended_voltage *= 10.0;
        //intended_voltage = map(intended_voltage, 0.0, 10.0, uni_min_output_voltage, total_range);

        float real_zero = zero_offset;
        float real_apex = 10.0 - apex_offset;
        Serial.printf("real_zero=%3.3f, ",    real_zero);
        Serial.printf("real_apex=%3.3f, ",    real_apex);
        Serial.printf("uni_max_output_voltage=%3.3f, ", uni_max_output_voltage);
        Serial.printf("apex_offset=%3.3f, ",    apex_offset);
        Serial.printf(" => ");

        intended_voltage = map(intended_voltage, 0.0, 10.0, real_zero, real_apex);

        return intended_voltage;
    }*/

   float get_calibrated_voltage(float intended_voltage) {
        // DIN = DDIN – OE – (FSE – OE) × (DDIN – 1024) ÷ 64512
        //intended_voltage = 10.0 - intended_voltage;
        uint32_t DDIN = (intended_voltage / 10.0) * 65535.0;
        uint32_t DIN = (DDIN - OE) - ((float)(FSE - OE) * (float)(DDIN - O_VALUE)) / (float)FS_VALUE;

        Serial.printf("get_calibrated_voltage(%3.3f): ", intended_voltage);

        Serial.printf("DDIN=%i, ", DDIN);
        Serial.printf("OE=%i, ", OE);
        Serial.printf("FSE=%i, ", FSE);
        Serial.printf("=> DIN=%i ", DIN);
        float result = (DIN / 65535.0) * 10.0;
        Serial.printf("=> %3.3f\n", result);

        return result;
   }

    void setup_cv_output() {
        dac_output.begin();

        menu->add_page("CV Output");
        /*menu->add(new ActionConfirmItem("Calibrate unipolar MIN", &calibrate_unipolar_minimum));
        menu->add(new ActionConfirmItem("Calibrate unipolar MAX", &calibrate_unipolar_maximum));
        menu->add(new ActionConfirmItem("Calibrate bipolar",  &calibrate_bipolar));*/
        menu->add(new ActionConfirmItem("Start calibrating", &start_calibration));

        /*
        menu->add(new DirectNumberControl<volatile float>("uni min", &uni_min_output_voltage, uni_min_output_voltage, -20.0, 20.0));
        menu->add(new DirectNumberControl<volatile float>("uni max", &uni_max_output_voltage, uni_max_output_voltage, 0.0, 20.0));
        menu->add(new DirectNumberControl<volatile float>("bi min", &bi_min_output_voltage, bi_min_output_voltage, -20.0, 20.0));
        menu->add(new DirectNumberControl<volatile float>("bi max", &bi_max_output_voltage, bi_max_output_voltage, 0.0, 20.0));
        */

       menu->add(new DirectNumberControl<int32_t>("OE",  &OE,  OE,  -16384, 16384));
       menu->add(new DirectNumberControl<int32_t>("FSE", &FSE, FSE, -16384, 16384));

    }
#endif