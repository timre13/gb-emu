#ifndef SERIAL_VIEWER_H
#define SERIAL_VIEWER_H

#include "config.h"
#include "common.h"

#include <SDL2/SDL.h>

#include <string>

#include "FontManager.h"

class SerialViewer final
{
private:
    SDL_Window      *m_window;
    SDL_Renderer    *m_renderer;
    FontManager     *m_fontManager;

    std::string     m_buffer;

    int m_winWChars{80};
    int m_winHChars{20};
    int m_scrollUpChars{};

    int m_fontW{};
    int m_fontH{};

public:
    SerialViewer(int x, int y);

    inline void write(const std::string string) { m_buffer += string; }
    inline void write(char character) { m_buffer += character; }

    inline void show() { SDL_ShowWindow(m_window); }
    inline void hide() { SDL_HideWindow(m_window); }

    inline void clearRenderer()
    {
        SDL_SetRenderDrawColor(m_renderer, 220, 220, 220, 255);
        SDL_RenderClear(m_renderer);
    }

    inline void updateRenderer() { SDL_RenderPresent(m_renderer); }

    void updateText();

    ~SerialViewer();
};

#endif /* SERIAL_VIEWER_H */
