#ifndef DEBUGWINDOW_H_
#define DEBUGWINDOW_H_

#include "config.h"

#include "common.h"

#include "CPU.h"
#include "Registers.h"
#include "Memory.h"

#include "TextRenderer.h"

#include <SDL2/SDL.h>

class DebugWindow final
{
private:
    SDL_Window      *m_window;
    SDL_Renderer    *m_renderer;
    std::unique_ptr<TextRenderer> m_textRend;
    std::string     m_content;

public:
    DebugWindow(FontLoader* fontLdr, int x, int y);

    inline void show() { SDL_ShowWindow(m_window); }
    inline void hide() { SDL_HideWindow(m_window); }

    inline void clearRenderer()
    {
        SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
        SDL_RenderClear(m_renderer);
    }

    inline void updateRenderer()
    {
        m_textRend->renderText(m_content);
        SDL_RenderPresent(m_renderer);
        m_textRend->endFrame();
        m_content.clear();
    }

    void updateRegisterValues(const Registers *registers);
    void updateOpcodeValue(const CPU *cpu);
    // Update values in memory and memory-mapped registers
    void updateMemoryValues(Memory *memory);

    ~DebugWindow();
};

#endif /* DEBUGWINDOW_H_ */
