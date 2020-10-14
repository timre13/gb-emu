#include "config.h"
#include "GBEmulator.h"

int main()
{
    GBEmulator *emulator{new GBEmulator};

    emulator->startLoop();

    delete emulator;
}
