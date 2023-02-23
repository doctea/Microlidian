
#include "midi/midi_usb.h"

// midihelpers library clock handling
#include <clock.h>
#include <bpm.h>

Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

void setup_usb() {
    #if defined(ARDUINO_ARCH_MBED) && defined(ARDUINO_ARCH_RP2040)
        // Manual begin() is required on core without built-in support for TinyUSB such as mbed rp2040
        TinyUSB_Device_Init(0);
    #endif

    while( !TinyUSBDevice.mounted() ) delay(1);
}

void setup_midi() {
    MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.turnThruOff();

    MIDI.setHandleClock(pc_usb_midi_handle_clock);
    MIDI.setHandleStart(pc_usb_midi_handle_start);
    MIDI.setHandleStop(pc_usb_midi_handle_stop);
    MIDI.setHandleContinue(pc_usb_midi_handle_continue);
}