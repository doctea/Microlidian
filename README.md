# Microlidian / μlidian

WIP.

Project for XIAO RP2040 mcu with st7789 screen and rotary encoder.  Using components and libraries from [usb_midi_clocker](https://github.com/doctea/usb_midi_clocker) project.  

The aim here is to have a flexible and extensible MIDI generator based on the mutating Euclidian features from [drum2musocv](https://github.com/doctea/drum2musocv), but aiming to be a bit more modular and extensible.

Eventually thinking of targeting an 8-10hp Eurorack panel with USB-C in, 3 bipolar CV inputs (using Pimoroni ADC board) with TRS MIDI out for sending 16+ rhythm tracks to a MIDIMUSO-CV12 board.

## Requirements

- [mymenu](https://github.com/doctea/mymenu)
- [midihelpers](https://github.com/doctea/midihelpers)

## TODO

- Implement a basic sequencer
 - with Euclidian stuff on top of that
 - menus for controlling such
 - make a nice graphical display widget for it
- MIDI input
- MIDI output
- Integrate CV inputs
 - for controlling parameters
 - maybe for clock too?
- Fix redrawing problem due to no framebuffer in the TFT_eSPI library (can we just use that buffer wrapper library?)

## Done

- Basic engine for BPM and internal/external clock (based on midihelpers/usb_midi_clocker code)
- Basic display and encoder for changing options

