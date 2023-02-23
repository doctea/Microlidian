#ifndef MIDI_USB__INCLUDED
#define MIDI_USB__INCLUDED

// MIDI + USB
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

extern Adafruit_USBD_MIDI usb_midi;
extern midi::MidiInterface<midi::SerialMIDI<Adafruit_USBD_MIDI>> MIDI;

void setup_midi();
void setup_usb();

#endif