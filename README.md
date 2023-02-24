# Microlidian / μlidian

WIP.

Project for XIAO RP2040 mcu with st7789 screen and rotary encoder.  Using components and libraries from [usb_midi_clocker](https://github.com/doctea/usb_midi_clocker) project.  

The aim here is to have a flexible and extensible MIDI generator based on the mutating Euclidian features from [drum2musocv](https://github.com/doctea/drum2musocv), but aiming to be a bit more modular and extensible.

Eventually thinking of targeting an 8-10hp Eurorack panel with USB-C in, 3 bipolar CV inputs (using Pimoroni ADC board) with TRS MIDI out for sending 16+ rhythm tracks to a MIDIMUSO-CV12 board.

## Requirements

- [mymenu](https://github.com/doctea/mymenu)
- [midihelpers](https://github.com/doctea/midihelpers)
- [parameters](httpa://github.com/doctea/parameters)
- vscode+platformio
- the gfx library by Bodmer for fast DMA writes

## Hardware/wiring

Clockwise starting from top-right:-

- 3.3v
- gnd
- 5v
- D10/MOSI -> ST7789 SPI
- D9/MISO  -> back button
- D8/SCK   -> ST7789 SCK
- D7/CS    -> ST7789 CS

- D6       -> UART MIDI out
- D5       -> i2c scl (for adc)
- D4       -> i2c sda (for adc)
- D3       -> Encoder right
- D2       -> Encoder left
- D1       -> Encoder button
- D0       -> ST7789 DC (probably swap this with D9?)

## TODO

- ~~Implement a basic sequencer~~
  - ~~with Euclidian stuff on top of that~~
  - menus for controlling such
    - modulate the parameters from cv or midi input 
  - ~~make a nice graphical display widget for it~~
- MIDI input (via USB, for setting options, ~~clock sync,~~ forwarding notes and drums)
  - options to enable/disable all this
- MIDI output (via TRS/DIN, for sending clock, triggers, notes and envelopes)
  - ~~basic hacky approach is working~~
  - options to enable/disable all this
- Integrate CV inputs
  - ~~basic cv input parameter~~
  - for controlling parameters
  - maybe for clock too?
- Figure out a way to provide storage, to save presets and templates/.mid files?  Need an extra pin for CS
  - use an i2c flashmem chip?; needs no extra pins; would need to support mounting it over usb in order to send files to it
  - use an attiny85 as a coprocessor that reads from the i2c and sends values over serial to the host: would also be useful for usb_midi_clocker or generally for these things; extra level of coding required; frees up 2xi2c pins, 1 of which would be needed for SD CS, so would have an extra pin free...
  - use an i2c rotary encoder: frees up 3(!) pins; could use the rgb encoder for extra feedback; more difficult to fit neatly under a panel due to the i2c board...
  - or, just use a Pico instead of the XIAO and don't worry about not having enough pins!; may be harder to fit usb-c connector, maybe not tho?
  - can we write to flash without it being overwritten when uploading new code?

## Done

- Basic engine for BPM and internal/external clock (based on midihelpers/usb_midi_clocker code)
- Basic display and encoder for changing options
- Fix redrawing problem due to no framebuffer in the TFT_eSPI library (can we just use that buffer wrapper library?)
