#include <clock.h>

void clock_mode_changed(ClockMode old_mode, ClockMode new_mode);
bool actual_check_cv_clock_ticked();

extern int time_between_cv_input_updates;
