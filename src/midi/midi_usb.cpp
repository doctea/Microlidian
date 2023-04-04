#include "Config.h"
#include "midi/midi_usb.h"

#include "debug.h"

// midihelpers library clock handling
#include <clock.h>
#include <bpm.h>

#include <SoftwareSerial.h>

Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, USBMIDI);

#ifdef MIDI_SERIAL_SOFTWARE
    #include <SoftwareSerial.h>
    SoftwareSerial SoftSerial(D6, -1, false);
    MIDI_CREATE_INSTANCE(SoftwareSerial, SoftSerial, DINMIDI);
#endif
#ifdef MIDI_SERIAL_SPIO
    SerialPIO spio(D6, SerialPIO::NOPIN);
    MIDI_CREATE_INSTANCE(SerialPIO, spio, DINMIDI);
#endif
#ifdef MIDI_SERIAL_HARDWARE
    MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, DINMIDI);
#endif

void setup_usb() {
    #if defined(ARDUINO_ARCH_MBED) && defined(ARDUINO_ARCH_RP2040)
        // Manual begin() is required on core without built-in support for TinyUSB such as mbed rp2040
        TinyUSB_Device_Init(0);
    #endif

    //while( !TinyUSBDevice.mounted() ) delay(1);
}

void auto_handle_start() {
    if(clock_mode != CLOCK_EXTERNAL_USB_HOST) {
        // automatically switch to using external USB clock if we receive a START message from the usb host
        // todo: probably move this into the midihelper library?
        //set_clock_mode(CLOCK_EXTERNAL_USB_HOST);
        if (__clock_mode_changed_callback!=nullptr)
            __clock_mode_changed_callback(clock_mode, CLOCK_EXTERNAL_USB_HOST);
        clock_mode = CLOCK_EXTERNAL_USB_HOST;
        //ticks = 0;
        messages_log_add("Auto-switched to CLOCK_EXTERNAL_USB_HOST");
    }
    pc_usb_midi_handle_start();
}

void setup_midi() {
    // setup USB MIDI connection
    USBMIDI.begin(MIDI_CHANNEL_OMNI);
    USBMIDI.turnThruOff();

    // callbacks for messages recieved from USB MIDI host
    USBMIDI.setHandleClock(pc_usb_midi_handle_clock);
    USBMIDI.setHandleStart(auto_handle_start); //pc_usb_midi_handle_start);
    USBMIDI.setHandleStop(pc_usb_midi_handle_stop);
    USBMIDI.setHandleContinue(pc_usb_midi_handle_continue);

    // setup serial MIDI output on standard UART pins; send only
    DINMIDI.begin(0);
    DINMIDI.turnThruOff();
}