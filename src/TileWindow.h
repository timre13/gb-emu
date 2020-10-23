#ifndef TILEWINDOW_H
#define TILEWINDOW_H

#include <SDL2/SDL.h>

#include "Memory.h"

class TileWindow final
{
private:
    SDL_Window      *m_window;
    SDL_Renderer    *m_renderer;

    int m_x{};
    int m_y{};
    int m_w{};
    int m_h{};

public:
    TileWindow(int x, int y);
    
    inline void clearRenderer()
    {
        SDL_SetRenderDrawColor(m_renderer, 220, 220, 220, 255);
        SDL_RenderClear(m_renderer);
    }

    inline void updateRenderer()
    {
        SDL_RenderPresent(m_renderer);
    }

    void updateTiles(Memory *memory);

    ~TileWindow();
};

#endif // TILEWINDOW_H