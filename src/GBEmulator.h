#ifndef GBEMULATOR_H_
#define GBEMULATOR_H_

#include "config.h"

#include "common.h"

#include "CPU.h"
#include "PPU.h"
#include "CartridgeReader.h"
#include "Memory.h"
#include "Joypad.h"
#include "Timer.h"

#include "DebugWindow.h"
#include "TileWindow.h"
#include "SerialViewer.h"

#include <string>
#include <SDL2/SDL.h>

class GBEmulator final
{
private:
    bool            m_isDone{};

    bool            m_isDebugWindowShown{};
    bool            m_isTileWindowShown{};
    bool            m_isSerialViewerShown{};

    SDL_Window      *m_window{nullptr};
    uint32_t        m_windowId{};
    SDL_Renderer    *m_renderer{nullptr};

    CPU             *m_cpu{nullptr}; // the registers are in the CPU
    PPU             *m_ppu{nullptr};
    Memory          *m_memory{nullptr};
    CartridgeReader *m_cartridgeReader{nullptr};
    Joypad          *m_joypad{nullptr};
    Timer           *m_timer{nullptr};


    CartridgeInfo   *m_cartridgeInfo{nullptr};

    DebugWindow     *m_debugWindow{nullptr};
    TileWindow      *m_tileWindow{nullptr};
    SerialViewer    *m_serialViewer{nullptr};

    std::string     m_romFilename;

    int             m_clockCyclesUntilPPUActivity{456};

    void initGUI();
    void initDebugWindow();
    void initTileWindow();
    void initSerialViewer();
    void initHardware();

    void deinit();
    
    void showCartridgeInfo();

    void updateGraphics();
    void emulateCycle();

    void waitForSpaceKey();

    void updateDebugWindow();
    void toggleDebugWindow();

    void updateTileWindow();
    void toggleTileWindow();

    void updateSerialViewer();
    void toggleSerialViewer();

public:
    GBEmulator(const std::string &romFilename);

    void startLoop();

    ~GBEmulator();
};

#endif /* GBEMULATOR_H_ */
