#include <clock.h>

void clock_mode_changed(ClockMode old_mode, ClockMode new_mode);
bool take_cv_clock_tick(uint32_t &observed_at_us);
