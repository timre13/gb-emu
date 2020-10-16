#include "config.h"
#include "GBEmulator.h"

int main()
{

    //GBEmulator *emulator{new GBEmulator{"roms/Pokemon - Blue Version (UE) [S][!].gb"}};
    //GBEmulator *emulator{new GBEmulator{"roms/Super Mario Land (JUE) (V1.1) [!].gb"}};
    //GBEmulator *emulator{new GBEmulator{"roms/Super Mario Bros. Deluxe (U) (V1.1) [C][!].gbc"}};
    //GBEmulator *emulator{new GBEmulator{"roms/cpu_instrs.gb"}};
    //GBEmulator *emulator{new GBEmulator{"roms/Tetris (JUE) (V1.1) [!].gb"}};
    GBEmulator *emulator{new GBEmulator{"roms/Dr. Mario (JU) (V1.1).gb"}};


    emulator->startLoop();

    delete emulator;
}
