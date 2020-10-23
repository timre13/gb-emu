#ifndef GBEMULATOR_H_
#define GBEMULATOR_H_

#include "config.h"

#include "CPU.h"
#include "CartridgeReader.h"
#include "Memory.h"

#include "DebugWindow.h"
#include "TileWindow.h"

#include <string>
#include <SDL2/SDL.h>

class GBEmulator final
{
private:
    bool            m_isDone{};

    SDL_Window      *m_window{nullptr};
    SDL_Renderer    *m_renderer{nullptr};

    CPU             *m_cpu{nullptr}; // the registers are in the CPU
    Memory          *m_memory{nullptr};
    CartridgeReader *m_cartridgeReader{nullptr};

    CartridgeInfo   *m_cartridgeInfo{nullptr};

    DebugWindow     *m_debugWindow{nullptr};
    TileWindow      *m_tileWindow{nullptr};

    std::string     m_romFilename;

    void initGUI();
    void initDebugWindow();
    void initTileWindow();
    void deinit();
    void initHardware();
    void showCartridgeInfo();

    void emulateCycle();

public:
    GBEmulator(const std::string &romFilename);

    void startLoop();

    ~GBEmulator();
};

#endif /* GBEMULATOR_H_ */
