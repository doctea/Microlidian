#include "core_safe.h"

//#include "pico/stdlib.h"
#include "hardware/sync.h"

std::atomic<bool> menu_locked = false;

uint32_t menu_interrupts = 0;

void acquire_lock() {
    {
        while(menu_locked) {};
        menu_interrupts = save_and_disable_interrupts();
        menu_locked = true;
    }
}
void release_lock() {
    if (is_locked())
        restore_interrupts(menu_interrupts);
    menu_locked = false;
}
bool is_locked() {
    return menu_locked;
}