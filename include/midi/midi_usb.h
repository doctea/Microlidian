#ifndef MIDI_USB__INCLUDED
#define MIDI_USB__INCLUDED

#include "Config.h"

// MIDI + USB
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

//usb midi
extern Adafruit_USBD_MIDI usb_midi;
extern midi::MidiInterface<midi::SerialMIDI<Adafruit_USBD_MIDI>> USBMIDI;

#ifdef MIDI_SERIAL_SPIO
    extern midi::MidiInterface<midi::SerialMIDI<SerialPIO>> DINMIDI;
#endif

#ifdef MIDI_SERIAL_HARDWARE
    extern midi::MidiInterface<midi::SerialMIDI<HardwareSerial>> DINMIDI;
#endif

#ifdef MIDI_SERIAL_SOFTWARE
    #include <SoftwareSerial.h>
    extern midi::MidiInterface<midi::SerialMIDI<SoftwareSerial>> DINMIDI;
#endif


void setup_midi();
void setup_usb();

#endif