# Microlidian / μlidian

WIP.

Project for XIAO RP2040 mcu with st7789 screen and rotary encoder.  Using components and libraries from [usb_midi_clocker](https://github.com/doctea/usb_midi_clocker) project.  

The aim here is to have a flexible and extensible MIDI generator based on the mutating Euclidian features from [drum2musocv](https://github.com/doctea/drum2musocv), but aiming to be a bit more modular and extensible.

8hp Eurorack panel with USB-Type C input for USB MIDI, 3 bipolar CV inputs (using Pimoroni ADC board), with TRS MIDI out for sending 16+ rhythm tracks to a MIDIMUSO-CV12 board (or other MIDI devices!).

## Requirements

- [mymenu](https://github.com/doctea/mymenu)
- [midihelpers](https://github.com/doctea/midihelpers)
- [parameters](httpa://github.com/doctea/parameters)
- vscode+platformio
- the gfx library by Bodmer for fast DMA writes

## Hardware/wiring

- WIP PCB & panels in KiCAD and Gerber format [here](https://github.com/doctea/Microlidian-hardware) - don't use these yet without checking with the author for any problems/new versions!

### Breadboard wiring

XIAO RP2040 pins.  Counting clockwise starting from top-right:-

- 3.3v
- gnd
- 5v
- D10/MOSI -> ST7789 SPI
- D9/MISO  -> back button (swapped with D0 on PCB version)
- D8/SCK   -> ST7789 SCK
- D7/CS    -> ST7789 CS

- D6       -> UART MIDI out
- D5       -> i2c scl (for adc)
- D4       -> i2c sda (for adc)
- D3       -> Encoder right
- D2       -> Encoder left
- D1       -> Encoder button
- D0       -> ST7789 DC (swapped with D9 on PCB version)

## TODO

- ~~Implement a basic sequencer~~
  - ~~with Euclidian stuff on top of that~~
  - menus for controlling such
    - modulate the parameters from cv or midi input 
  - ~~make a nice graphical display widget for it~~
- MIDI input (via USB, for setting options, ~~clock sync,~~ forwarding notes and drums)
  - options to enable/disable all this
- MIDI output (~~via TRS/DIN, for sending clock, triggers, notes~~ and triggerable envelopes/LFO)
  - ~~basic hacky approach is working~~
  - standard gm drum machine option with assignable drum numbers
  - midimuso cv trigger+cv modes
  - nicer mechanisms for assigning outputs
  - options that would allow for modulation 
  - options to enable/disable all this
- Integrate CV inputs
  - ~~basic cv input parameter~~
  - for controlling parameters
  - low-memory version of the mapping tool, since providing options for all options of 20 tracks uses 100K+ and takes us over the available RAM!
  - ~~maybe for clock too?~~
  - implement CV input 'reset'
- Figure out a way to provide storage, to save presets and templates/.mid files?  Need an extra pin for CS
  - use an i2c flashmem chip?; needs no extra pins; would need to support mounting it over usb in order to send files to it
  - use an attiny85 as a coprocessor that reads from the i2c and sends values over serial to the host: would also be useful for usb_midi_clocker or generally for these things; extra level of coding required; frees up 2xi2c pins, 1 of which would be needed for SD CS, so would have an extra pin free...
  - use an i2c rotary encoder: frees up 3(!) pins; could use the rgb encoder for extra feedback; more difficult to fit neatly under a panel due to the i2c board...
  - or, just use a Pico instead of the XIAO and don't worry about not having enough pins!; may be harder to fit usb-c connector, maybe not tho?
  - can we write to flash without it being overwritten when uploading new code?
- Stutter/Ratchetting
- Per-track duration, with modulation
- Setting a per-parameter range for modulation
- Load/save settings and presets to the flash
- "Stick on this pattern" control; A/B controls
- Logic gates on the triggers (including 'mult-limbed-drummer' options)
- 'Density' control that keeps the same rhythm but fills in gaps
  - 'euclidian within euclidian'?  so eg we fill in triggers in between other triggers?
- rhythm-synced LFO that sets a cycle of the waveform to the length between triggers
- Accept CCs to configure settings (similar to how drum2musocv already does it, perhaps)


## Done

- Basic engine for BPM and internal/external clock (based on midihelpers/usb_midi_clocker code)
- Basic display and encoder for changing options
- Fix redrawing problem due to no framebuffer in the TFT_eSPI library (can we just use that buffer wrapper library?)
- Named MIDI outputs ("kick", "snare", etc, "bassline")
- Menu options and hotkeys to force reboot + firmware update modes:
  - Hold BACK button for 4 seconds to reboot (useful if the screen hasn't started up after a cold boot, or similar)
  - Hold BACK button for 4 seconds while holding encoder button for 3 seconds to go into firmware update mode
