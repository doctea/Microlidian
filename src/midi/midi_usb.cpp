
#include "midi/midi_usb.h"

// midihelpers library clock handling
#include <clock.h>
#include <bpm.h>

Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, USBMIDI);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, DINMIDI);

void setup_usb() {
    #if defined(ARDUINO_ARCH_MBED) && defined(ARDUINO_ARCH_RP2040)
        // Manual begin() is required on core without built-in support for TinyUSB such as mbed rp2040
        TinyUSB_Device_Init(0);
    #endif

    while( !TinyUSBDevice.mounted() ) delay(1);
}

void setup_midi() {

    // setup USB MIDI connection
    USBMIDI.begin(MIDI_CHANNEL_OMNI);
    USBMIDI.turnThruOff();

    USBMIDI.setHandleClock(pc_usb_midi_handle_clock);
    USBMIDI.setHandleStart(pc_usb_midi_handle_start);
    USBMIDI.setHandleStop(pc_usb_midi_handle_stop);
    USBMIDI.setHandleContinue(pc_usb_midi_handle_continue);

    // setup serial MIDI output on standard UART pins
    DINMIDI.turnThruOff();
    DINMIDI.begin(0);
}