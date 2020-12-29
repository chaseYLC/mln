#pragma once

#include <atomic>

namespace mln {
    class Spinlock
    {
    public:
        Spinlock() : m_state{ ATOMIC_FLAG_INIT } {}

        void lock()
        {
            while (m_state.test_and_set(std::memory_order_acquire)) {
            }
        }

        void unlock()
        {
            m_state.clear(std::memory_order_release);
        }
    private:
        std::atomic_flag m_state;
    };
}