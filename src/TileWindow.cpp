#include "TileWindow.h"

#include <stdint.h>
#include <iostream>

#include "Logger.h"
#include "string_formatting.h"

//#define TILE_WIN_USE_PALETTE
#define TILE_WIN_DRAW_TILE_SEP
#define TILE_WIN_TILES_PER_ROW 16
#define TILE_WIN_SCALE 4
#define TILE_WIN_W (TILE_WIN_TILES_PER_ROW*TILE_SIZE*TILE_WIN_SCALE)
#define TILE_WIN_H (128*3/TILE_WIN_TILES_PER_ROW*TILE_SIZE*TILE_WIN_SCALE)

TileWindow::TileWindow(int x, int y)
{
    m_window = SDL_CreateWindow("Tile Viewer", x, y, TILE_WIN_W, TILE_WIN_H, SDL_WINDOW_HIDDEN);
    if (!m_window) Logger::fatal("Failed to create window for Tile Window");

    m_renderer = SDL_CreateRenderer(m_window, -1, 0);
    if (!m_renderer) Logger::fatal("Failed to create renderer for Tile Window");
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(m_renderer, 220, 220, 220, 255);
    SDL_RenderClear(m_renderer);
}

void TileWindow::updateTiles(PPU *ppu, uint8_t lcdc)
{
#ifndef TILE_WIN_USE_PALETTE
    static constexpr uint8_t shades[]{
        255, // 0 - white
        200, // 1 - light gray
        100, // 2 - dark gray
        0    // 3 - black
    };
#endif

    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
    SDL_RenderClear(m_renderer);

    for (int tileI{}; tileI < NUM_OF_TILES; ++tileI)
    {
        for (int tilePixelI{}; tilePixelI < PIXELS_PER_TILE; ++tilePixelI)
        {
            auto colorI{ppu->getPixelColorIndexFlat(tileI, tilePixelI)};

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

    // Draw lines between the blocks
    SDL_SetRenderDrawColor(m_renderer, 0, 255, 0, 255);
    SDL_RenderDrawLine(m_renderer, 0, (128/TILE_WIN_TILES_PER_ROW)*TILE_SIZE*TILE_WIN_SCALE,
                          TILE_WIN_W, (128/TILE_WIN_TILES_PER_ROW)*TILE_SIZE*TILE_WIN_SCALE);
    SDL_RenderDrawLine(m_renderer, 0, (128/TILE_WIN_TILES_PER_ROW)*TILE_SIZE*TILE_WIN_SCALE*2,
                          TILE_WIN_W, (128/TILE_WIN_TILES_PER_ROW)*TILE_SIZE*TILE_WIN_SCALE*2);

    // Highlight the active blocks
    SDL_SetRenderDrawColor(m_renderer, 0, 255, 0, 30);
    SDL_Rect activeAreaRect{0, 0, TILE_WIN_W, 128*2/TILE_WIN_TILES_PER_ROW*TILE_SIZE*TILE_WIN_SCALE};
    if ((lcdc & LCDC_BIT_BG_WIN_TILE_DATA_AREA) == 0)
        activeAreaRect.y = 128/TILE_WIN_TILES_PER_ROW*TILE_SIZE*TILE_WIN_SCALE;
    SDL_RenderFillRect(m_renderer, &activeAreaRect);
}

TileWindow::~TileWindow()
{
    SDL_DestroyWindow(m_window);
    SDL_DestroyRenderer(m_renderer);
}

