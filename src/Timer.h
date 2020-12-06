#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

class Timer final
{
private:
    int m_cyclesUntilDivIncrement{256};
    uint8_t m_divRegister{};

    int m_cyclesUntilTimaIncrement{1024};
    uint8_t m_timaRegister{};
    uint8_t m_tmaRegister{}; // Value to set after TIMA overflows
    uint8_t m_tacRegister{}; // Timer control

    bool m_isInterruptRequested{};

public:
    Timer();

    void tick();

    // ----- DIV register ------
    inline uint8_t getDivRegister() const { return m_divRegister; }
    inline void resetDivRegister() { m_divRegister = 0; }

    // ------- Timer -----------
    inline void setTimaRegister(uint8_t value) { m_timaRegister = value; }
    inline uint8_t getTimaRegister() const { return m_timaRegister; }

    inline void setTmaRegister(uint8_t value) { m_tmaRegister = value; }
    inline uint8_t getTmaRegister() const { return m_tmaRegister; }

    inline void setTacRegister(uint8_t value) { m_tacRegister = value; }
    inline uint8_t getTacRegister() const { return m_tacRegister; }

    inline bool isInterruptRequested() const { return m_isInterruptRequested; }
    inline void resetInterrupt() { m_isInterruptRequested = false; }

    ~Timer();
};

#endif // TIMER_H
