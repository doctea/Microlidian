#include <Arduino.h>

#include "debug.h"

bool debug_flag = false;

LinkedList<String> *messages_log = new LinkedList<String>();

void messages_log_add(String msg) {
  messages_log->add(msg);
  if (messages_log->size() >= MAX_MESSAGES_LOG) {
    messages_log->unlink(0);
  }
}

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

  void reset_rp2040 () {
    // https://forums.raspberrypi.com/viewtopic.php?t=318747
    //#define AIRCR_Register (*((volatile uint32_t*)(PPB_BASE + 0x0ED0C)))
    //AIRCR_Register = 0x5FA0004;
    watchdog_reboot(0,0,0);
  }

  #include "mymenu.h"
  #include "mymenu/screen.h"

  void reset_upload_firmware() {
    while(menu_locked) {}; // wait until current screen drawing on other core has finished
    menu_locked = true;    // lock so that other core won't trash what we're about to draw to the screen
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