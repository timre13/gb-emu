#include "Registers.h"

Registers::Registers()
{
    // Set the initial values
    setAF(0x01b0);
    setBC(0x0013);
    setDE(0x00d8);
    setHL(0x014d);
    setSP(0xfffe);
    setPC(0x0100);
}
