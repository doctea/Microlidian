
#include "midi/midi_usb.h"

// midihelpers library clock handling
#include <clock.h>
#include <bpm.h>

#include <SoftwareSerial.h>

//SoftwareSerial SoftSerial(D6, -1, false);

SerialPIO spio(D6, SerialPIO::NOPIN);

Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, USBMIDI);
//MIDI_CREATE_INSTANCE(SoftwareSerial, SoftSerial, DINMIDI);
MIDI_CREATE_INSTANCE(SerialPIO, spio, DINMIDI);

void setup_usb() {
    #if defined(ARDUINO_ARCH_MBED) && defined(ARDUINO_ARCH_RP2040)
        // Manual begin() is required on core without built-in support for TinyUSB such as mbed rp2040
        TinyUSB_Device_Init(0);
    #endif

    while( !TinyUSBDevice.mounted() ) delay(1);
}

void setup_midi() {

    //Serial1.begin(31250);

    // setup USB MIDI connection
    USBMIDI.begin(MIDI_CHANNEL_OMNI);
    USBMIDI.turnThruOff();

    USBMIDI.setHandleClock(pc_usb_midi_handle_clock);
    USBMIDI.setHandleStart(pc_usb_midi_handle_start);
    USBMIDI.setHandleStop(pc_usb_midi_handle_stop);
    USBMIDI.setHandleContinue(pc_usb_midi_handle_continue);

    // setup serial MIDI output on standard UART pins
    /*Serial1.setRX(SerialPIO::NOPIN);
    Serial1.setTX(D6);
    Serial1.begin(31250);*/

    //serialDINMIDI.begin();

    DINMIDI.begin(0);
    DINMIDI.turnThruOff();
}