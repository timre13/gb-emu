#ifndef TILEWINDOW_H
#define TILEWINDOW_H

#include "config.h"

#include "common.h"

#include <SDL2/SDL.h>

#include "PPU.h"

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

    inline void updateRenderer()
    {
        SDL_RenderPresent(m_renderer);
    }

    void updateTiles(PPU *ppu);

    ~TileWindow();
};

#endif // TILEWINDOW_H
