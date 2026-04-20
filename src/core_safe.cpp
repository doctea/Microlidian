#include "core_safe.h"

//#define LOCK_USE_SPINLOCKS
//#define LOCK_NO_LOCKING
#define LOCK_MUTEX
//#define ATOMIC_WITH_INTERRUPTS

#ifdef LOCK_USE_SPINLOCKS
    #include "hardware/sync.h"
    #include "pico/multicore.h"

    spin_lock_t *menu_locked = spin_lock_init(spin_lock_claim_unused(true));
    uint32_t menu_interrupts = 0;

    void acquire_lock() {
        menu_interrupts = spin_lock_blocking(menu_locked);
    }
    void release_lock() {
        spin_unlock(menu_locked, menu_interrupts);
    }
    bool is_locked() {
        return is_spin_locked(menu_locked);
    }
#elif defined(LOCK_NO_LOCKING)
    // this crashes really quickly, proving that the atomic route does at least do something
    void acquire_lock() {}
    void release_lock() {}
    bool is_locked() { 
        return false; 
    }
#elif defined(LOCK_MUTEX)
    #include "pico/sync.h"
    #include <atomic>

    struct Mutex {
        recursive_mutex_t mutex;
        std::atomic<uint32_t> depth = 0;
        Mutex() {
            recursive_mutex_init(&mutex);
        }
        public:
        void lock() {
            recursive_mutex_enter_blocking(&mutex);
            depth.fetch_add(1, std::memory_order_relaxed);
        }
        void unlock() {
            const uint32_t current = depth.load(std::memory_order_relaxed);
            if (current > 0) {
                depth.store(current - 1, std::memory_order_relaxed);
            }
            recursive_mutex_exit(&mutex);
        }
        bool is_locked() {
            return depth.load(std::memory_order_relaxed) > 0;
        }
    };

    Mutex menu_locked;

    void acquire_lock() {
        menu_locked.lock();
    }
    void release_lock() {
        menu_locked.unlock();
    }
    bool is_locked() { 
        return menu_locked.is_locked(); 
    }
#else
    #include "hardware/sync.h"

    #include <atomic>
    std::atomic<bool> menu_locked = false;
    #ifdef ATOMIC_WITH_INTERRUPTS
        uint32_t menu_interrupts = 0;
    #endif


    void acquire_lock() {
        while(menu_locked) {};
        #ifdef ATOMIC_WITH_INTERRUPTS
            menu_interrupts = save_and_disable_interrupts();
        #endif
        menu_locked = true;
    }

    void release_lock() {
        menu_locked = false; 
        #ifdef ATOMIC_WITH_INTERRUPTS
            if (is_locked())
                restore_interrupts(menu_interrupts);
        #endif
    }

    bool is_locked() {
        return menu_locked;
    }
#endif