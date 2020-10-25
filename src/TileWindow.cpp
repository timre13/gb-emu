#include "TileWindow.h"

#include <stdint.h>
#include <iostream>

#include "Logger.h"
#include "string_formatting.h"

//#define TILE_WIN_USE_PALETTE
#define TILE_WIN_TILES_PER_ROW 16
#define TILE_WIN_SCALE 5

TileWindow::TileWindow(int x, int y)
{
    m_window = SDL_CreateWindow("Tile Viewer", x, y, 16*8*5, 16*8*5, 0);
    if (!m_window) Logger::fatal("Failed to create window for Tile Window");

    m_renderer = SDL_CreateRenderer(m_window, -1, 0);
    if (!m_renderer) Logger::fatal("Failed to create renderer for Tile Window");
}

void TileWindow::updateTiles(PPU *ppu)
{
#ifndef TILE_WIN_USE_PALETTE
    static constexpr uint8_t shades[]{
        255, // 0 - white
        200, // 1 - light gray
        100, // 2 - dark gray
        0    // 3 - black
    };
#endif

    for (int tileI{}; tileI < NUM_OF_TILES; ++tileI)
    {
        for (int tilePixelI{}; tilePixelI < PIXELS_PER_TILE; ++tilePixelI)
        {
            auto colorI{ppu->getPixelColorIndex(tileI, tilePixelI)};

#ifdef TILE_WIN_USE_PALETTE
            uint8_t r, g, b;
            ppu->getColorFromIndex(colorI, &r, &g, &b);

            SDL_SetRenderDrawColor(m_renderer, r, g, b, 255);
#else
            SDL_SetRenderDrawColor(m_renderer, shades[colorI], shades[colorI], shades[colorI], 255);
#endif

            auto pixelX{tileI%TILE_WIN_TILES_PER_ROW*TILE_SIZE*TILE_WIN_SCALE+tilePixelI%TILE_SIZE*TILE_WIN_SCALE};
            auto pixelY{tileI/TILE_WIN_TILES_PER_ROW*TILE_SIZE*TILE_WIN_SCALE+tilePixelI/TILE_SIZE*TILE_WIN_SCALE};

            SDL_Rect pixelRect{
                    pixelX,
                    pixelY,
                    TILE_WIN_SCALE,
                    TILE_WIN_SCALE};
            SDL_RenderFillRect(m_renderer, &pixelRect);
        }
    }
}

TileWindow::~TileWindow()
{
    SDL_DestroyWindow(m_window);
    SDL_DestroyRenderer(m_renderer);
}

