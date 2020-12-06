#include "Timer.h"

Timer::Timer()
{
}

void Timer::tick()
{
    // ------- Handle DIV register --------

    --m_cyclesUntilDivIncrement;
    if (m_cyclesUntilDivIncrement <= 0)
    {
        m_cyclesUntilDivIncrement = 256;
        ++m_divRegister;
    }

    // --------- Handle timer --------

    // TODO: Implement these:
    // https://gbdev.io/pandocs/#timer-obscure-behaviour

    // If the timer is disabled, don't do anything with it.
    if ((m_tacRegister & 0b00000100) == 0) return;

    --m_cyclesUntilTimaIncrement;
    if (m_cyclesUntilTimaIncrement <= 0)
    {
        // If the TIMA overflows
        if (m_timaRegister == 0xff)
        {
            // Set it to TMA
            m_timaRegister = m_tmaRegister;
            // Request the timer interrupt
            m_isInterruptRequested = true;
        }
        else
        {
            ++m_timaRegister;
        }

        // Count down the cycles until increment from the value of TAC.
        switch (m_tacRegister & 0b00000011)
        {
        case 0:
            m_cyclesUntilDivIncrement = 1024;
            break;
        case 1:
            m_cyclesUntilDivIncrement = 16;
            break;
        case 2:
            m_cyclesUntilDivIncrement = 64;
            break;
        case 3:
            m_cyclesUntilDivIncrement = 256;
            break;
        }
    }
}

Timer::~Timer()
{
}

