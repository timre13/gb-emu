#include "PPU.h"

#include "Logger.h"
#include "string_formatting.h"

#include <iostream>

// Draw a grid between pixels
//#define PPU_DRAW_GRID
// Ignore Background Palette Register
//#define PPU_IGNORE_BPR

PPU::PPU(SDL_Renderer *renderer, Memory *memory)
    : m_rendererPtr{renderer}, m_memoryPtr{memory}
{
}

uint8_t PPU::getPixelColorIndex(int tileI, int tilePixelI) const
{
    uint16_t pixelDataAddress{(uint16_t)(TILE_RAM_START+tileI*TILE_SIZE*2+tilePixelI/TILE_SIZE*2)};
    uint8_t colorI{
            (uint8_t)
            ((m_memoryPtr->get(pixelDataAddress+0, false) & (1 << (TILE_SIZE-tilePixelI%TILE_SIZE-1))) ? 2 : 0 |
             (m_memoryPtr->get(pixelDataAddress+1, false) & (1 << (TILE_SIZE-tilePixelI%TILE_SIZE-1))) ? 1 : 0)};

    return colorI;
}

void PPU::getColorFromIndex(uint8_t index, uint8_t *rOut, uint8_t *gOut, uint8_t *bOut)
{
    // Get the value of the Background Palette Register
    const uint8_t bgpValue{m_memoryPtr->get(REGISTER_ADDR_BGP, false)};

    static constexpr uint8_t palette[][3]{
            {0x82, 0x78, 0x0d},
            {0x5c, 0x71, 0x22},
            {0x3a, 0x53, 0x36},
            {0x1c, 0x36, 0x28}
    };

    // Get which color is mapped to the color index
    auto paletteEntryI{(bgpValue & (3 << index*2)) >> index*2};

    *rOut = palette[paletteEntryI][0];
    *gOut = palette[paletteEntryI][1];
    *bOut = palette[paletteEntryI][2];
}

void PPU::updateBackground()
{
    if (m_memoryPtr->get(REGISTER_ADDR_LY, false) > 153) // End of V-BLANK
    {
        m_currentBgMapByteI = 0;

        m_memoryPtr->set(REGISTER_ADDR_LY, 0);
    }

    if (m_memoryPtr->get(REGISTER_ADDR_LY, false) < 144) // Not V-BLANK
    {
        // Draw a tile
        for (int pixelI{}; pixelI < PIXELS_PER_TILE; ++pixelI)
        {
            uint8_t r, g, b;

#ifdef PPU_IGNORE_BPR
            static constexpr uint8_t shades[]{
                255, // 0 - white
                200, // 1 - light gray
                100, // 2 - dark gray
                0    // 3 - black
            };

            auto colorI{getPixelColorIndex(m_memoryPtr->get(BG_MAP_START+m_currentBgMapByteI, false), pixelI)};
            r = shades[colorI]; g = shades[colorI]; b = shades[colorI];
#else
            getColorFromIndex(getPixelColorIndex(m_memoryPtr->get(BG_MAP_START+m_currentBgMapByteI, false), pixelI), &r, &g, &b);
#endif

            SDL_SetRenderDrawColor(m_rendererPtr, r, g, b, 255);

            SDL_Rect pixelRect{
                m_currentBgMapByteI%BG_MAP_TILES_PER_ROW*TILE_SIZE*PIXEL_SCALE+pixelI%TILE_SIZE*PIXEL_SCALE,
                m_currentBgMapByteI/BG_MAP_TILES_PER_ROW*TILE_SIZE*PIXEL_SCALE+pixelI/TILE_SIZE*PIXEL_SCALE,
                PIXEL_SCALE,
                PIXEL_SCALE};
            SDL_RenderFillRect(m_rendererPtr, &pixelRect);

#ifdef PPU_DRAW_GRID
        SDL_SetRenderDrawColor(m_rendererPtr, 255, 0, 0, 255);
        SDL_RenderDrawRect(m_rendererPtr, &pixelRect);
#endif
        }

        ++m_currentBgMapByteI;

    }

    m_memoryPtr->set(REGISTER_ADDR_LY, m_memoryPtr->get(REGISTER_ADDR_LY, false)+1);
}
