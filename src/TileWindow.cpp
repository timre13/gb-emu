#include "TileWindow.h"

#include <stdint.h>
#include <iostream>

#include "Logger.h"
#include "string_formatting.h"

TileWindow::TileWindow(int x, int y)
{
    m_window = SDL_CreateWindow("Tile Viewer", x, y, 16*8*5, 16*8*5, 0);
    if (!m_window) Logger::fatal("Failed to create window for Tile Window");

    m_renderer = SDL_CreateRenderer(m_window, -1, 0);
    if (!m_renderer) Logger::fatal("Failed to create renderer for Tile Window");
}

void TileWindow::updateTiles(Memory *memory)
{
    /*
    static constexpr uint8_t palette[][3]{
            {87, 98, 81},
            {50, 76, 42},
            {13, 41, 33},
            { 0,  7, 10}
    };
    */

    static constexpr uint8_t shades[]{
        255,
        200,
        100,
        0
    };

    static constexpr int tileSize{8};
    static constexpr int tilesPerLine{16};
    static constexpr int pixelScale{5};
    static constexpr int numOfTiles{384};
    static constexpr int tileRamStart{0x8000};

    for (int tileI{}; tileI < numOfTiles; ++tileI)
    {
        for (int tilePixelI{}; tilePixelI < tileSize*tileSize; ++tilePixelI)
        {
            uint8_t colorI{
                    (uint8_t)
                    ((memory->get(tileRamStart+tileI*tileSize*2+0+tilePixelI/tileSize, false) & (1 << (tileSize-tilePixelI%tileSize))) ? 2 : 0 |
                     (memory->get(tileRamStart+tileI*tileSize*2+1+tilePixelI/tileSize, false) & (1 << (tileSize-tilePixelI%tileSize))) ? 1 : 0)};

            SDL_SetRenderDrawColor(m_renderer, shades[colorI], shades[colorI], shades[colorI], 255);
            SDL_Rect pixelRect{
                    tileI%tilesPerLine*tileSize*pixelScale+tilePixelI%tileSize*pixelScale,
                    tileI/tilesPerLine*tileSize*pixelScale+tilePixelI/tileSize*pixelScale,
                    pixelScale,
                    pixelScale};
            SDL_RenderFillRect(m_renderer, &pixelRect);
        }
    }
}

TileWindow::~TileWindow()
{
    SDL_DestroyWindow(m_window);
    SDL_DestroyRenderer(m_renderer);
}

