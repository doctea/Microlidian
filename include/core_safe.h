

#include <atomic>
extern std::atomic<bool> menu_locked;

void acquire_lock();
void release_lock();
bool is_locked();