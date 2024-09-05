#ifndef FLASHMEM
    #define FLASHMEM
    //#define F(x) { x }
#endif

#ifdef ENABLE_SCREEN
    #ifdef TFT_BODMER
        #define DisplayTranslator_Configured DisplayTranslator_Bodmer
    #endif
    #ifdef TFT_ST7789
        #define DisplayTranslator_Configured DisplayTranslator_ST7789
    #endif
    //#ifndef TFT_ST7789_T3
    //    #define TFT_ST7789_T3
    //    #define TFT_ST7789_T3_BIG
    //#endif
    #define MENU_MS_BETWEEN_REDRAW  20
#endif

/*#ifndef ENABLE_SCREEN
    #define tft_print(X) { Serial.println(X) }
#endif*/

#ifdef ENABLE_SCREEN
    #define ENCODER_STEP_DIVISOR    4
    #define PIN_BUTTON_A    D1   // pin to receive encoder button
    #ifdef BUILD_BREADBOARD
        #define PIN_BUTTON_B    D9   // pin to receive back button    //;; breadboard version has DC on D0, and back button on D9
        //#define PIN_BUTTON_C    26  // pin to receive right-hand / save button
        #define ENCODER_KNOB_L  D2   // pin to receive left-encoder pulses
        #define ENCODER_KNOB_R  D3   // pin to receive right-encoder pulses
    #endif
    #ifdef BUILD_PCB
        #define PIN_BUTTON_B    D0   // pin to receive back button
        //#define PIN_BUTTON_C    26  // pin to receive right-hand / save button
        #define ENCODER_KNOB_L  D3   // pin to receive left-encoder pulses
        #define ENCODER_KNOB_R  D2   // pin to receive right-encoder pulses
    #endif
#endif

// default for using MidiMUSO CV12 mappings on the DIN output
#define DEFAULT_OUTPUT_TYPE     OUTPUT_TYPE::DRUMS_MIDIMUSO

// default to using straight MIDI output on the DIN output, eg for connecting directly to a drum machine
//#define DEFAULT_OUTPUT_TYPE     OUTPUT_TYPE::NORMAL

//#define DEFAULT_CLOCK_MODE  CLOCK_EXTERNAL_USB_HOST      

/*#define SCREEN_WIDTH 135
#define SCREEN_HEIGHT 240
#define SCREEN_ROTATION 3

#define TFT_SCLK D8
#define TFT_MOSI D10*/

#define MIDI_SERIAL_OUT_PIN D6

#define ENABLE_CV_INPUT 0x48
//#define ENABLE_PARAMETER_MAPPING

//#define ENABLE_DEBUG_SERIAL

// defined in platformio.ini build flags
//#define ENABLE_STORAGE
//#define ENABLE_CALIBRATION_STORAGE  
//#define LOAD_CALIBRATION_ON_BOOT
//#define ENABLE_EUCLIDIAN

#define TIME_BETWEEN_CV_INPUT_UPDATES 5

#define MUTATE_EVERY_TICK

//#define Serial_println(X) { if (Serial) Serial.println(x); }

// choose what type of rp2040 serial driver to use on the DIN MIDI output
// this is specified in the platformio.ini because it needs to be picked up by the midihelpers midi_usb_rp2040.cpp file
//#define MIDI_SERIAL_SOFTWARE    // this disnae work at all?  
//#define MIDI_SERIAL_HARDWARE    // THIS FUCKIN WORKS LADS!!!
//#define MIDI_SERIAL_SPIO        // this fuckin works too!  

//#define ENABLE_ENVELOPES
#define DEFAULT_ENVELOPE_CLASS Weirdolope
//#define DEFAULT_ENVELOPE_CLASS RegularEnvelope

//#define PROCESS_USB_ON_SECOND_CORE