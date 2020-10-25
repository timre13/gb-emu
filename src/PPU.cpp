#include "PPU.h"

#include "Logger.h"
#include "string_formatting.h"

#include <iostream>

//#define PPU_DRAW_GRID

PPU::PPU(SDL_Renderer *renderer, Memory *memory)
    : m_rendererPtr{renderer}, m_memoryPtr{memory}
{
}

uint8_t PPU::getPixelColorIndex(int tileI, int tilePixelI) const
{
    uint16_t pixelDataAddress{(uint16_t)(TILE_RAM_START+tileI*TILE_SIZE*2+0+tilePixelI/TILE_SIZE)};
    uint8_t colorI{
            (uint8_t)
            ((m_memoryPtr->get(pixelDataAddress+0, false) & (1 << (TILE_SIZE-tilePixelI%TILE_SIZE))) ? 2 : 0 |
             (m_memoryPtr->get(pixelDataAddress+1, false) & (1 << (TILE_SIZE-tilePixelI%TILE_SIZE))) ? 1 : 0)};

    return colorI;
}

void PPU::getColorFromIndex(uint8_t index, uint8_t *rOut, uint8_t *gOut, uint8_t *bOut)
{
    // Get the value of the Background Palette Register
    const uint8_t bgpValue{m_memoryPtr->get(0xff47, false)};

    static constexpr uint8_t palette[][3]{
            {87, 98, 81},
            {50, 76, 42},
            {13, 41, 33},
            { 0,  7, 10}
    };

    // Get which color is mapped to the color index
    auto paletteEntryI{(bgpValue & (3 << index*2)) >> index*2};

    //std::cout << toBinStr(paletteEntryI, 8, false) << std::endl;

    *rOut = palette[paletteEntryI][0];
    *gOut = palette[paletteEntryI][1];
    *bOut = palette[paletteEntryI][2];
}

void PPU::updateBackground()
{
    if (m_memoryPtr->get(0xff44, false) > 153) // End of V-BLANK
    {
        m_currentBgMapByteI = 0;

        m_memoryPtr->set(0xff44, 0);
    }

    if (m_memoryPtr->get(0xff44, false) < 144) // Not V-BLANK
    {
        //if (BG_MAP_START+m_currentBgMapByteI > BG_MAP_START+BG_MAP_TILES_PER_ROW*BG_MAP_TILES_PER_COL-1)

        // Draw a tile
        for (int pixelI{}; pixelI < PIXELS_PER_TILE; ++pixelI)
        {
            uint8_t r, g, b;
            getColorFromIndex(getPixelColorIndex(m_currentBgMapByteI, pixelI), &r, &g, &b);

            SDL_SetRenderDrawColor(m_rendererPtr, r, g, b, 255);

            SDL_Rect pixelRect{
                m_currentBgMapByteI%BG_MAP_TILES_PER_ROW*TILE_SIZE*PIXEL_SCALE+pixelI%TILE_SIZE*PIXEL_SCALE,
                m_currentBgMapByteI/BG_MAP_TILES_PER_ROW*TILE_SIZE*PIXEL_SCALE+pixelI/TILE_SIZE*PIXEL_SCALE,
                PIXEL_SCALE,
                PIXEL_SCALE};
            SDL_RenderFillRect(m_rendererPtr, &pixelRect);
        }

#ifdef PPU_DRAW_GRID
        SDL_SetRenderDrawColor(m_rendererPtr, 255, 0, 0, 255);
        SDL_RenderDrawRect(m_rendererPtr, &pixelRect);
#endif

        ++m_currentBgMapByteI;

    }

    m_memoryPtr->set(0xff44, m_memoryPtr->get(0xff44, false)+1);
}
