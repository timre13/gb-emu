#include "TileWindow.h"

#include <stdint.h>
#include <iostream>

#include "Logger.h"
#include "string_formatting.h"

//#define TILE_WIN_USE_PALETTE
#define TILE_WIN_TILES_PER_ROW 16
#define TILE_WIN_SCALE 3

TileWindow::TileWindow(int x, int y)
{
    m_window = SDL_CreateWindow("Tile Viewer", x, y, TILE_WIN_TILES_PER_ROW*TILE_SIZE*TILE_WIN_SCALE, 16*TILE_SIZE*TILE_WIN_SCALE*2, SDL_WINDOW_HIDDEN);
    if (!m_window) Logger::fatal("Failed to create window for Tile Window");

    m_renderer = SDL_CreateRenderer(m_window, -1, 0);
    if (!m_renderer) Logger::fatal("Failed to create renderer for Tile Window");

    SDL_SetRenderDrawColor(m_renderer, 220, 220, 220, 255);
    SDL_RenderClear(m_renderer);
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

    for (int tileRamI{}; tileRamI <= 2; ++tileRamI)
    {
        for (int tileI{}; tileI < NUM_OF_TILES; ++tileI)
        {
            for (int tilePixelI{}; tilePixelI < PIXELS_PER_TILE; ++tilePixelI)
            {
                auto colorI{ppu->getPixelColorIndex(tileI, tilePixelI, static_cast<PPU::TileDataSelector>(tileRamI))};

#ifdef TILE_WIN_USE_PALETTE
                uint8_t r, g, b;
                ppu->getColorFromIndex(colorI, &r, &g, &b);

                SDL_SetRenderDrawColor(m_renderer, r, g, b, 255);
#else
                SDL_SetRenderDrawColor(m_renderer, shades[colorI], shades[colorI], shades[colorI], 255);
#endif

                auto pixelX{tileI%TILE_WIN_TILES_PER_ROW*TILE_SIZE*TILE_WIN_SCALE+tilePixelI%TILE_SIZE*TILE_WIN_SCALE};
                auto pixelY{tileI/TILE_WIN_TILES_PER_ROW*TILE_SIZE*TILE_WIN_SCALE+tilePixelI/TILE_SIZE*TILE_WIN_SCALE+
                            tileRamI*16*TILE_SIZE*TILE_WIN_SCALE};

                SDL_Rect pixelRect{
                        pixelX,
                        pixelY,
                        TILE_WIN_SCALE,
                        TILE_WIN_SCALE};
                SDL_RenderFillRect(m_renderer, &pixelRect);
            }
        }

        // Draw a line after the first tile set
        if (!tileRamI)
        {
            SDL_SetRenderDrawColor(m_renderer, 0, 255, 0, 255);
            for (int i{-1}; i <= 1; ++i)
                SDL_RenderDrawLine(
                        m_renderer,
                        0, 16*TILE_SIZE*TILE_WIN_SCALE+i,
                        TILE_WIN_TILES_PER_ROW*TILE_SIZE*TILE_WIN_SCALE, 16*TILE_SIZE*TILE_WIN_SCALE+i);
        }
    }
}

TileWindow::~TileWindow()
{
    SDL_DestroyWindow(m_window);
    SDL_DestroyRenderer(m_renderer);
}

