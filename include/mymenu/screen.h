#ifndef SCREEN__INCLUDED
#define SCREEN__INCLUDED

// input libraries
#include "Encoder.h"
#include "Bounce2.h"

#include "mymenu.h"

void setup_screen();
void push_display();
void update_screen_dontcare();
bool update_screen();

extern Encoder encoder;
extern Bounce pushButton;

#include <atomic>
extern std::atomic<bool> ticked;
extern std::atomic<bool> started;


#endif