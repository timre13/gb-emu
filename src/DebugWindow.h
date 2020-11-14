#ifndef DEBUGWINDOW_H_
#define DEBUGWINDOW_H_

#include "config.h"

#include "common.h"

#include "CPU.h"
#include "Registers.h"
#include "Memory.h"

#include "FontManager.h"

#include <SDL2/SDL.h>

class DebugWindow final
{
private:
    SDL_Window      *m_window;
    SDL_Renderer    *m_renderer;
    FontManager     *m_fontManager;

    int m_x{};
    int m_y{};
    int m_w{};
    int m_h{};

    int m_fontW{};
    int m_fontH{};

    void renderText(const std::string &string, int x, int y, uint8_t colorR=0, uint8_t colorG=0, uint8_t colorB=0);

public:
    DebugWindow(int x, int y);

    inline void show() { SDL_ShowWindow(m_window); }
    inline void hide() { SDL_HideWindow(m_window); }

    inline void clearRenderer()
    {
        SDL_SetRenderDrawColor(m_renderer, 220, 220, 220, 255);
        SDL_RenderClear(m_renderer);
    }

    inline void updateRenderer() { SDL_RenderPresent(m_renderer); }

    void updateRegisterValues(const Registers *registers);
    void updateOpcodeValue(const CPU *cpu);
    // Update values in memory and memory-mapped registers
    void updateMemoryValues(const Memory *memory);

    ~DebugWindow();
};

#endif /* DEBUGWINDOW_H_ */
