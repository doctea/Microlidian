# Microlidian / μlidian

WIP.

Project for XIAO RP2040 mcu with st7789 screen and rotary encoder.  Using components and libraries from [usb_midi_clocker](https://github.com/doctea/usb_midi_clocker) project.  

The aim here is to have a flexible and extensible MIDI generator based on the mutating Euclidian features from [drum2musocv](https://github.com/doctea/drum2musocv), but aiming to be a bit more modular and extensible.

8hp Eurorack panel with USB-Type C input for USB MIDI, 3 bipolar CV inputs (using Pimoroni ADC board), with TRS MIDI out for sending 16+ rhythm tracks to a MIDIMUSO-CV12 board (or other MIDI devices!).

## Requirements

- [mymenu](https://github.com/doctea/mymenu)
- [midihelpers](https://github.com/doctea/midihelpers)
- [parameters](httpa://github.com/doctea/parameters)
- [patched Encoder library](https://github.com/doctea/Encoder) 
- [Vortigont LinkedList library](https://github.com/vortigont/LinkedList)
- vscode+platformio
- the gfx library by Bodmer for fast DMA writes

## Hardware/PCB wiring

- Use env:pcb with -DBUILD_PCB
- WIP PCB & panels in KiCAD and Gerber format [here](https://github.com/doctea/Microlidian-hardware) - don't use these yet without checking with the author for any problems/new versions!

### Breadboard wiring

- Use env:pcb with -DBUILD_BREADBOARD

Seeed XIAO RP2040 pins.  Counting clockwise starting from top-right:-

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
    - modulate the parameters from ~~cv or~~ midi input 
  - ~~make a nice graphical display widget for it~~
- Other sequencer types
  - eg raga sequencer that loads .mid files
- MIDI input (via USB, for setting options, ~~clock sync,~~ forwarding notes and drums)
  - options to enable/disable all this
- MIDI output (~~via TRS/DIN, for sending clock, triggers, notes~~ and ~triggerable envelopes~~/LFO)
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
    - worked around for now by not allowing the sources for a slot to be changed
  - ~~maybe for clock too?~~
  - implement CV input 'reset'
  - Setting a per-parameter range for modulation
- MIDI inputs for controlling parameters
  - Configurable MIDI inputs for control surfaces from host USB
  - Accept baked-in CCs to configure settings (similar to how drum2musocv already does it, perhaps)
- Figure out a way to provide storage, to save presets and templates/.mid files?
  - ~~just save to the internal flash via LittleFS!~~ DONE
  - ~~rudimentary save/load of parameter mappings~~ DONE
  - allow configuration of the output devices/channels..
  - add other settings+things to be saved/recalled..?
  - Load/save settings and presets to the flash
- Other sequencer types
  - eg raga sequencer that loads .mid files
- Free up pins..?
  - use an i2c rotary encoder: frees up 3(!) pins; could use the rgb encoder for extra feedback; more difficult to fit neatly under a panel due to the i2c board...
  - or, just use a Pico or Waveshare Zero instead of the XIAO and don't worry about not having enough pins!
- Stutter/Ratchetting
- Per-track note duration, with modulation
- "Stick on the current pattern" control; A/B controls
- Optional logic gates on the triggers (including 'mult-limbed-drummer' options)
- an enhanced 'Density' control that keeps the same rhythm, but fills in gaps?
  - 'euclidian within euclidian'?  so eg we fill in triggers in between other triggers?
- rhythm-synced LFO that sets a cycle of the waveform to the length between triggers
- Ability to boot up as a USB flash drive, for copying files to/from the LittleFS flash?
- Allow holding a button to go into firmware update mode before running much code at all as a failsafe against bad code that won't complete setup() or first loop()

### Known problems/gotchas

- Trying to Serial.print() from the second core (screen+CV input) while serial is connected will hang the display!
- Trying to Serial.print() generally seems to be unreliable and likely to cause crashes
- Occasional glitching of output name display when on some track pages; intermittent crashes bizarrely happening when rendering output labels on pattern page; ~~probably caused by something overwriting selected output slot or something similar?~~ ????
  - worked around this by caching the label in OutputSelectorControl, but will probably still cause crashes if displaying a list of Output names
- Slow loading presets causing pauses and problems
- we don't seem to get around to doing a menu update_ticks() for every tick, so the ParameterInputDisplay graph appears very noisy unless backfilling values.  even updating it from a callback for every parameter input read() didnt seem to be fast enough.  so might mean that very fast changes are being missed..

## Done

- Basic engine for BPM and internal/external clock (based on midihelpers/usb_midi_clocker code)
- Basic display and encoder for changing options
- Fix redrawing problem due to no framebuffer in the TFT_eSPI library (can we just use that buffer wrapper library?)
- Named MIDI outputs ("kick", "snare", etc, "bassline")
- Menu options and hotkeys to force reboot + firmware update modes:
  - Hold BACK button for 4 seconds to reboot (useful if the screen hasn't started up after a cold boot, or similar)
  - Hold BACK button for 4 seconds while holding encoder button for 3 seconds to go into firmware update mode
- FIXED: Screen doesn't turn on first time on the revision 1 hardware, don't know why.  Hold the 'back' button for 4+ seconds to force a reboot and it will usually start working!
  - Tried: adding a 47k pullup resistor from CS->BLK
  - Tried: adding a delay before setup
  - Tried+fixed: added pullup resistor to display RES pin

