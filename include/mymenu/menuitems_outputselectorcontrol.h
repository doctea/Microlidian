#ifndef OUTPUTSELECTOR_MENUITEMS__INCLUDED
#define OUTPUTSELECTOR_MENUITEMS__INCLUDED

#include "menuitems.h"

#include "outputs/output.h"

//class BaseOutput;

// Selector to choose a ParameterInput from the available list to use a Source; used by objects/parameters that can only feed from one ParameterInput at a time, eg CVInput
template<class TargetClass> //,class BaseOutput>
class OutputSelectorControl : public SelectorControl<int> {
    BaseOutput *initial_selected_object = nullptr;
    LinkedList<BaseOutput*> *available_objects = nullptr;

    TargetClass *target_object = nullptr;
    void(TargetClass::*setter_func)(BaseOutput*) = nullptr;

    bool show_values = false;   // whether to display the incoming values or not

    public:

    OutputSelectorControl(
        const char *label, 
        TargetClass *target_object, 
        void(TargetClass::*setter_func)(BaseOutput*), 
        BaseOutput*(TargetClass::*getter_func)(), 
        LinkedList<BaseOutput*> *available_objects,
        BaseOutput *initial_selected_object = nullptr,
        bool show_values = false
    ) : SelectorControl(label, 0) {
        this->show_values = show_values;
        this->initial_selected_object = initial_selected_object;    // hot damn .. 
        this->available_objects = available_objects;
        this->target_object = target_object;
        this->setter_func = setter_func;
        this->num_values = available_objects->size();
    };

    virtual void configure (LinkedList<BaseOutput*> *available_objects) {
        this->available_objects = available_objects;
        const char *initial_name = (char*)"None";
        if (this->initial_selected_object!=nullptr)
            initial_name = this->initial_selected_object->label;
        actual_value_index = this->find_index_for_label(initial_name);
    }

    virtual int find_index_for_label(const char *name)  {
        if (this->available_objects==nullptr)
            return -1;
        unsigned const int size = this->available_objects->size();
        for (unsigned int i = 0 ; i < size ; i++) {
            if (available_objects->get(i)->matches_label(name))
                return i;
        }
        return -1;
        //return parameter_manager->getInputIndexForName(name);
    }

    virtual int find_index_for_object(BaseOutput *input)  {
        return this->find_index_for_label(input->label);
    }

    virtual void on_add() override {
        //this->debug = true;
        //Serial.printf(F("%s#on_add...\n"), this->label); Serial_flush();
        actual_value_index = -1;
        /*if (this->debug) {
            Serial.printf("on_add() in ParameterSelectorControl @%p:\n", this); Serial_flush();
            Serial.printf("\tParameterSelectorControl with initial_selected_parameter @%p...\n", initial_selected_parameter_input); Serial_flush();
            if (initial_selected_parameter_input!=nullptr) {
                Serial.printf("\tParameterSelectorControl looking for '%s' @%p...\n", initial_selected_parameter_input->label, initial_selected_parameter_input); Serial_flush();
            } else 
                Serial.println("\tno initial_selected_parameter set");
        }*/

        if (initial_selected_object!=nullptr) {
            //Serial.printf(F("%s#on_add: got non-null initial_selected_parameter_input\n")); Serial_flush();
            //Serial.printf(F("\tand its name is %c\n"), initial_selected_parameter_input->name); Serial_flush();
            //this->actual_value_index = parameter_manager->getInputIndexForName(initial_selected_parameter_input->name); ////this->find_parameter_input_index_for_label(initial_selected_parameter_input->name);
            this->actual_value_index = this->find_index_for_label(initial_selected_object->label);
        } else {
            this->actual_value_index = -1;
        }
        this->selected_value_index = this->actual_value_index;
        //Serial.printf(F("#on_add returning"));
    }

    char label_for_index[MAX_LABEL];
    virtual const char *get_label_for_index(int index)  {
        //static char label_for_index[MENU_C_MAX];
        // todo: this is currently unused + untested
        if (index<0 || index >= this->available_objects->size())
            return "None";
        snprintf(label_for_index, MAX_LABEL, "%s", this->available_objects->get(index)->label);
        return label_for_index;
    }

    // update the control to reflect changes to selection (eg, called when new value is loaded from project file)
    virtual void update_source(BaseOutput *new_source)  {
        //int index = parameter_manager->getPitchInputIndex(new_source);
        //Serial.printf("update_source got index %i\n", index);
        int index = this->find_index_for_object(new_source);
        this->update_actual_index(index);
    }

    virtual void setter(int new_value) override {
        //if (this->debug) Serial.printf(F("ParameterSelectorControl changing from %i to %i\n"), this->actual_value_index, new_value);
        selected_value_index = actual_value_index = new_value;
        if(new_value>=0 && new_value<this->available_objects->size() && this->target_object!=nullptr && this->setter_func!=nullptr) {
            (this->target_object->*this->setter_func)(this->available_objects->get(new_value));
        }
    }
    virtual int getter() override {
        return selected_value_index;
    }

    // classic fixed display version
    virtual int display(Coord pos, bool selected, bool opened) override {
        //Serial.println(F("ParameterInputSelectorControl display()!")); Serial_flush();
        tft->setTextSize(0);

        //pos.y = header(label, pos, selected, opened);
      
        num_values = this->available_objects->size(); //NUM_AVAILABLE_PARAMETERS;
        //Serial.printf(F("\tdisplay got num_values %i\n"), num_values); Serial_flush();

        if (!opened) {
            // not selected, so just show the current value on one row
            //Serial.printf("\tnot opened\n"); Serial_flush();
            colours(selected, this->default_fg, BLACK);

            //tft->setTextSize((strlen(txt) < max_character_width/2) ? 2 : 1);
            tft->setTextSize(2);

            if (this->actual_value_index>=0 && this->actual_value_index < num_values) {
                //Serial.printf(F("\tactual value index %i\n"), this->actual_value_index); Serial_flush();
                tft->printf((char*)"Output: %s\n", (char*)this->get_label_for_index(this->actual_value_index));
                //Serial.printf(F("\tdrew selected %i\n"), this->actual_value_index); Serial_flush();
            } else {
                tft->printf((char*)"Output: none\n");
            }
        } else {
            // opened, so show the possible values to select from
            const int current_value = actual_value_index; //this->getter();
            if (selected_value_index==-1) 
                selected_value_index = actual_value_index;
            const int start_value = tft->will_x_rows_fit_to_height(selected_value_index, tft->height()-pos.y) ? 0 : selected_value_index;
            
            for (unsigned int i = start_value ; i < num_values ; i++) {
                //Serial.printf("%s#display() looping over parameterinput number %i of %i...\n", this->label, i, this->available_parameter_inputs->size()); Serial.flush();
                const BaseOutput *param_input = this->available_objects->get(i);
                if (param_input==nullptr) {
                    tft->println("??null??");
                    continue;
                }
                //Serial.printf("%s#display() got param_input %p...", param_input); Serial.flush();
                //Serial.printf("named %s\n", param_input->name); Serial.flush();
                const bool is_current_value_selected = i==current_value;
                const int col = is_current_value_selected ? GREEN : C_WHITE; //param_input->colour;
                colours(opened && selected_value_index==i, col, BLACK);

                //tft->printf("%s\n", (char*)param_input->name);
                tft->println((const char*)param_input->label);

                if (tft->getCursorY()>(tft->height())) 
                    break;
            }
            if (tft->getCursorX()>0) // if we haven't wrapped onto next line then do it manually
                tft->println(); //(char*)"\n");
        }
        return tft->getCursorY();
    }

    int crash_flag = 0;
    BaseOutput *last_found = nullptr;

    virtual int renderValue(bool selected, bool opened, uint16_t max_character_width) override {
        const int index_to_display = opened ? selected_value_index : actual_value_index;
        const int col = selected_value_index==this->actual_value_index && opened ? 
                GREEN : C_WHITE;
                //(index_to_display>=0 ? this->available_objects->get(index_to_display)->colour : YELLOW/2);
        
        colours(selected, col, BLACK);
        char txt[MAX_LABEL];
        if (index_to_display>=0 && index_to_display<this->available_objects->size()) {
            BaseOutput *obj = this->available_objects->get(index_to_display);
            if (last_found!=nullptr && obj!=last_found) {
                crash_flag++;
            }
            // todo: sprintf to correct number of max_character_width characters
            if (crash_flag) {
                tft->printf("index %i, crash_flag %i!!\n", index_to_display, crash_flag);
                snprintf(txt, MAX_LABEL, "%p changed to ", last_found);
                tft->println(txt);
                snprintf(txt, MAX_LABEL, "%p (diff %i)", obj, (last_found-obj));
                tft->setTextSize(2);
                tft->println(txt);
                last_found = obj;
                return tft->getCursorY();
            } else {
                last_found = obj;
            }
            snprintf(txt, MAX_LABEL, "%6s", this->get_label_for_index(index_to_display));
        } else {
            snprintf(txt, MAX_LABEL, "None");
        }
        if (!crash_flag) 
            tft->setTextSize((strlen(txt) < max_character_width/2) ? 2 : 1);
        tft->println(txt);
        return tft->getCursorY();
    }

    virtual bool button_select() override {
        //Serial.printf("button_select with selected_value_index %i\n", selected_value_index);
        //Serial.printf("that is available_values[%i] of %i\n", selected_value_index, available_values[selected_value_index]);
        this->setter(selected_value_index);

        char msg[MENU_MESSAGE_MAX];
        //Serial.printf("about to build msg string...\n");
        const char *name = this->get_label_for_index(selected_value_index);
        //if (selected_value_index>=0)
        snprintf(msg, MENU_MESSAGE_MAX, "Set %s to %s (%i)", label, name, selected_value_index);
        //Serial.printf("about to set_last_message!");
        //msg[20] = '\0'; // limit the string so we don't overflow set_last_message
        menu_set_last_message(msg,GREEN);

        return go_back_on_select;
    }

    virtual void set_available_values(LinkedList<BaseOutput*> *available_values) {
        this->available_objects = available_values;
        this->num_values = this->available_objects->size();
    }

};

#endif