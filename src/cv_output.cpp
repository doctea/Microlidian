#ifdef ENABLE_CV_OUTPUT
    #include "cv_output.h"

    #include "menu.h"
    #include "menuitems_action.h"

    #include "cv_input.h"

    #include "SimplyAtomic.h"
    #include "core_safe.h"

    DAC8574 *dac_output; //(ENABLE_CV_OUTPUT);

    bool cv_output_enabled = true;

    bool calibrating = false;

    void start_calibration() {
        calibrating = true;
    }

    //volatile float uni_max_output_voltage = 9.557, uni_min_output_voltage = -0.331;
    //volatile float uni_max_output_voltage = 9.557, uni_min_output_voltage = 0.331;
    //volatile float uni_max_output_voltage = 9.53, uni_min_output_voltage = 0.33;
    //volatile float bi_max_output_voltage = uni_max_output_voltage,  bi_min_output_voltage = uni_min_output_voltage;

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

    /*void calibrate_unipolar_minimum(int channel, char *input_name) {
        //calibrating = true;

        uni_min_output_voltage = (10.0 * (calibrate_find_dac_value_for(channel, input_name, 0.0) / 65535.0));
        //uni_min_output_voltage *= -1.0;
        Serial.printf("calibrate_unipolar_minimum found %3.3f\n", uni_min_output_voltage);
    }

    void calibrate_unipolar_maximum(int channel, char *input_name) {
        uni_max_output_voltage = 10.0 * (calibrate_find_dac_value_for(channel, input_name, 10.0) / 65535.0);

        //uni_max_output_voltage = 10.0 + (10.0 - uni_max_output_voltage);
        Serial.printf("calibrate_unipolar_maximum found %3.3f\n", uni_max_output_voltage);
    }*/

    /*void calibrate_unipolar_maximum() {
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
    }*/

    /*void calibrate_unipolar_minimum() {
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
        float zero_offset = uni_min_output_voltage; // * -65535.0;
        float apex_offset = uni_max_output_voltage;
        //float total_range = uni_max_output_voltage - uni_min_output_voltage;
        //float total_range = uni_max_output_voltage - zero_offset + apex_offset;

        //intended_voltage += zero_offset;
        //intended_voltage /= 10.0;
        //intended_voltage *= (total_range/10.0);
        //intended_voltage *= 10.0;
        //intended_voltage = map(intended_voltage, 0.0, 10.0, uni_min_output_voltage, total_range);

        float real_zero = zero_offset;
        float real_apex = apex_offset;
        //Serial.printf("real_zero=%3.3f, ",    real_zero);
        //Serial.printf("real_apex=%3.3f, ",    real_apex);
        //Serial.printf("uni_max_output_voltage=%3.3f, ", uni_max_output_voltage);
        //Serial.printf("apex_offset=%3.3f, ",    apex_offset);
        //Serial.printf(" => \n");

        intended_voltage = map(intended_voltage, 0.0, 10.0, real_zero, real_apex);

        return intended_voltage;
    }*/

   /*float get_calibrated_voltage(float intended_voltage) {
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
   }*/
  

    #include "parameters/CVOutputParameter.h"
    #include "mymenu_items/ParameterMenuItems_lowmemory.h"

    void setup_cv_output() {
        dac_output = new DAC8574(ENABLE_CV_OUTPUT);
        dac_output->begin();

        CVOutputParameter<DAC8574,float> *output_a = new CVOutputParameter<DAC8574,float>("CVO-A", dac_output, 0, VALUE_TYPE::UNIPOLAR, true);
        CVOutputParameter<DAC8574,float> *output_b = new CVOutputParameter<DAC8574,float>("CVO-B", dac_output, 1, VALUE_TYPE::UNIPOLAR, true);
        CVOutputParameter<DAC8574,float> *output_c = new CVOutputParameter<DAC8574,float>("CVO-C", dac_output, 2, VALUE_TYPE::UNIPOLAR, true);
        CVOutputParameter<DAC8574,float> *output_d = new CVOutputParameter<DAC8574,float>("CVO-D", dac_output, 3, VALUE_TYPE::UNIPOLAR, true);

        output_a->set_parameter_input_for_calibration((VoltageParameterInput*)parameter_manager->getInputForName("A"));
        output_b->set_parameter_input_for_calibration((VoltageParameterInput*)parameter_manager->getInputForName("B"));
        output_c->set_parameter_input_for_calibration((VoltageParameterInput*)parameter_manager->getInputForName("C"));

        /*LinkedList<FloatParameter*> list = new LinkedList<FloatParameter*>();
        list->add(output_a);
        list->add(output_b);
        list->add(output_c);
        list->add(output_d);*/

        parameter_manager->addParameter(output_a);
        parameter_manager->addParameter(output_b);
        parameter_manager->addParameter(output_c);
        parameter_manager->addParameter(output_d);

        menu->add_page("CVO-A");
        create_low_memory_parameter_controls(output_a->label, output_a);
        //menu->add(parameter_manager->makeMenuItemsForParameter(output_a));
        menu->add_page("CVO-B");
        create_low_memory_parameter_controls(output_b->label, output_b);
        //menu->add(parameter_manager->makeMenuItemsForParameter(output_b));
        menu->add_page("CVO-C");
        create_low_memory_parameter_controls(output_c->label, output_c);
        //menu->add(parameter_manager->makeMenuItemsForParameter(output_c));
        menu->add_page("CVO-D");
        create_low_memory_parameter_controls(output_d->label, output_d);
        //menu->add(parameter_manager->makeMenuItemsForParameter(output_d));

    //}

    //void setup_cv_output_menu() {
        /*
        menu->add_page("CV Output");
        menu->add(new ActionConfirmItem("Start calibrating", &start_calibration));
      
        menu->add(new DirectNumberControl<volatile float>("uni min", &uni_min_output_voltage, uni_min_output_voltage, -20.0, 20.0));
        menu->add(new DirectNumberControl<volatile float>("uni max", &uni_max_output_voltage, uni_max_output_voltage, 0.0, 20.0));
        */
    }
#endif