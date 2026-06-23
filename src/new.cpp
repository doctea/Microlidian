
#include <stdlib.h>

// #include "wiring.h"

// Replacement functions that use EXTMEM instead of RAM2 for new/delete

// with thanks to KurtE and Beermat 
// https://forum.pjrc.com/index.php?threads/putting-objects-instantiated-with-new-into-extmem-instead-of-ram2.76731/

#include "Arduino.h"


#ifdef ENABLE_PSRAM

#warning "ENABLE_PSRAM is defined, so new/delete will use EXTMEM instead of RAM2.  Make sure you have a PSRAM chip connected to the RP2350!"

#include "psram.h"

#define RESERVE_RAM2 65536        // keep 64k of RAM2 reserved
//#define RESERVE_RAM2 32768        // keep 32k of RAM2 reserved
extern unsigned long _heap_start;
extern unsigned long _heap_end;
extern char *__brkval;

void * operator new(size_t size)
{
    if (rp2040.getFreeHeap() > RESERVE_RAM2+size) {
        void * new_obj = malloc(size);
        if (new_obj) return new_obj;
    }
    return pmalloc(size);
}

void * operator new[](size_t size)
{
    if (rp2040.getFreeHeap() > RESERVE_RAM2+size) {
        void * new_obj = malloc(size);
        if (new_obj) return new_obj;
    }
    return pmalloc(size);
}

void operator delete(void * ptr)
{
    if ((uint32_t) ptr >= 0x70000000) {
        free(ptr);
    } else {
        free(ptr);
    }
}

void operator delete[](void * ptr)
{
    if ((uint32_t) ptr >= 0x70000000) {
        free(ptr);
    } else {
        free(ptr);
    }
}

void operator delete(void * ptr, size_t size)
{
    if ((uint32_t) ptr >= 0x70000000) {
        free(ptr);
    } else {
        free(ptr);
    }
}

void operator delete[](void * ptr, size_t size)
{
    if ((uint32_t) ptr >= 0x70000000) {
        free(ptr);
    } else {
        free(ptr);
    }
}

#endif