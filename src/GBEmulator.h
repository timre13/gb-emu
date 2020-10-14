#ifndef GBEMULATOR_H_
#define GBEMULATOR_H_

#include "config.h"

#include "CPU.h"
#include "CartridgeReader.h"
#include "Memory.h"

#include "DebugWindow.h"

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

    void initGUI();
    void initDebugWindow();
    void deinit();
    void initHardware();
    void showCartridgeInfo();

    void emulateCycle();

public:
    GBEmulator();

    void startLoop();

    ~GBEmulator();
};

#endif /* GBEMULATOR_H_ */
