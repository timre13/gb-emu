#ifndef SERIAL_VIEWER_H
#define SERIAL_VIEWER_H

#include "config.h"
#include "common.h"

#include <SDL2/SDL.h>

#include <string>

#include "TextRenderer.h"

class SerialViewer final
{
private:
    SDL_Window      *m_window;
    SDL_Renderer    *m_renderer;
    std::unique_ptr<TextRenderer> m_textRend;

    std::string     m_buffer;

    static constexpr int winWChars = 80;
    static constexpr int winHChars = 20;

public:
    SerialViewer(FontLoader* fontLdr, int x, int y);

    inline void write(const std::string string) { m_buffer += string; }
    inline void write(char character) { if (character) m_buffer += character; }

    inline void show() { SDL_ShowWindow(m_window); }
    inline void hide() { SDL_HideWindow(m_window); }

    inline void clearRenderer()
    {
        SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
        SDL_RenderClear(m_renderer);
    }

    inline void updateRenderer()
    {
        SDL_RenderPresent(m_renderer);
        m_textRend->endFrame();
    }

    void updateText();

    ~SerialViewer();
};

#endif /* SERIAL_VIEWER_H */
