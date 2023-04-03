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

#define DEFAULT_CLOCK_MODE  CLOCK_EXTERNAL_USB_HOST      

#define SCREEN_WIDTH 135
#define SCREEN_HEIGHT 240
#define SCREEN_ROTATION 3

#define TFT_SCLK D8
#define TFT_MOSI D10

//#define ENABLE_DEBUG_SERIAL
#define ENABLE_STORAGE

#define ENABLE_CV_INPUT 0x48
//#define ENABLE_PARAMETER_MAPPING
/*
// defined in platformio.ini build flags
#define ENABLE_CALIBRATION_STORAGE  
#define LOAD_CALIBRATION_ON_BOOT
*/
#define TIME_BETWEEN_CV_INPUT_UPDATES 25

#define ENABLE_EUCLIDIAN
#define MUTATE_EVERY_TICK

//#define Serial_println(X) { if (Serial) Serial.println(x); }