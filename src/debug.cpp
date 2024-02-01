#include <Arduino.h>

#include "debug.h"

#if defined(__arm__) && defined(CORE_TEENSY)
  extern unsigned long _heap_start;
  extern unsigned long _heap_end;
  extern char *__brkval;

  int freeRam() {
    return (char *)&_heap_end - __brkval;
  }

  void debug_free_ram() {
    //Serial.println(F("debug_free_ram() not implemented on Teensy"));
    if (Serial) Serial.printf(F("debug_free_ram: %i\n"), freeRam());
  }

  FLASHMEM void reset_teensy() {
      // https://forum.pjrc.com/threads/57810-Soft-reboot-on-Teensy4-0
      #define CPU_RESTART_ADDR (uint32_t *)0xE000ED0C
      #define CPU_RESTART_VAL 0x5FA0004
      #define CPU_RESTART (*CPU_RESTART_ADDR = CPU_RESTART_VAL);
      if (Serial) Serial.println(F("Restarting!\n")); Serial_flush();
      CPU_RESTART;
      //Serial.println(F("Restarted?!"); Serial_flush();
  }
#elif defined(__avr__)
  int freeRam () {  
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
  }
  void debug_free_ram() {
    if (Serial) Serial.print(F("Free RAM is "));
    if (Serial) Serial.println(freeRam());
  }
#elif defined(ARDUINO_ARCH_RP2040)
  #include "pico/stdlib.h"  // not sure if we need this?
  #include "pico/bootrom.h" // needed for reset_usb_boot

  #ifdef USE_UCLOCK
    #include "uClock.h"
  #endif

  #include "mymenu.h"
  #include "mymenu/screen.h"
  #include "core_safe.h"

  void reset_rp2040 () {
    #ifdef USE_UCLOCK
      uClock.stop();
    #endif
    watchdog_reboot(0,0,0);
  }

  void reset_upload_firmware() {
    #ifdef USE_UCLOCK
      uClock.stop();
    #endif
    acquire_lock();// lock so that other core won't trash what we're about to draw to the screen
    if (menu!=nullptr && menu->tft) {
      menu->tft->clear();
      menu->tft->setTextSize(3);
      menu->tft->setCursor(0,0);
      menu->tft->setTextColor(BLACK, C_WHITE);
      menu->tft->println("   FIRMWARE  \n    UPDATE   ");
      menu->tft->setTextSize(2);
      menu->tft->setTextColor(C_WHITE, BLACK);
      menu->tft->println("1.Connect USB to PC\n2.Open mounted drive\n3.Copy new .uf2 file\n4.Profit");
      menu->tft->updateDisplay();
    }
    delay(100);
    reset_usb_boot(0,0);
  }

  int freeRam () {  
    return rp2040.getFreeHeap();
  }
  void debug_free_ram() {
    if (Serial) Serial.print(F("Free RAM: "));
    if (Serial) Serial.println(freeRam());
  }
#else
  int freeRam () {  
    return 1337
  }
  void debug_free_ram() {
    if (Serial) Serial.print(F("Free RAM: not implemented on this platform"));
    if (Serial) Serial.println(freeRam());
  }
#endif

volatile int menu_time = 10;
volatile int tft_time = 10;
//int missed_micros = 10;
volatile int sequencer_time = 10;