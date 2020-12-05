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

uint8_t PPU::getPixelColorIndex(int tileI, int tilePixelI, TileDataSelector bgDataSelector) const
{
    uint16_t pixelDataAddress{(uint16_t)((bgDataSelector == TileDataSelector::Lower ? TILE_DATA_L_START : TILE_DATA_H_START)+tileI*TILE_SIZE*2+tilePixelI/TILE_SIZE*2)};
    uint8_t colorI{
            (uint8_t)
            (((m_memoryPtr->get(pixelDataAddress+0, false) & (1 << (TILE_SIZE-tilePixelI%TILE_SIZE-1))) ? 2 : 0) |
             ((m_memoryPtr->get(pixelDataAddress+1, false) & (1 << (TILE_SIZE-tilePixelI%TILE_SIZE-1))) ? 1 : 0))};

    return colorI;
}

void PPU::getColorFromIndex(uint8_t index, uint8_t *rOut, uint8_t *gOut, uint8_t *bOut)
{
    // Get the value of the Background Palette Register
    const uint8_t bgpValue{m_memoryPtr->get(REGISTER_ADDR_BGP, false)};

    static constexpr uint8_t palette[][3]{
            {0x82, 0x78, 0x0d},
            {0x3a, 0x53, 0x36},
            {0x5c, 0x71, 0x22},
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
    const uint8_t lcdcRegValue{m_memoryPtr->get(REGISTER_ADDR_LCDC, false)};

    // If the LCD and PPU are disabled
    if ((lcdcRegValue & 0b10000000) == 0)
        return;

    const uint8_t lyRegValue{m_memoryPtr->get(REGISTER_ADDR_LY, false)};

    if (lyRegValue == 144) // Start of V-BLANK
    {
        // Call the V-blank interrupt
        m_memoryPtr->set(REGISTER_ADDR_IF, m_memoryPtr->get(REGISTER_ADDR_IF, false) | INTERRUPT_MASK_VBLANK, false);
    }

    if (lyRegValue > 153) // End of V-BLANK
    {
        m_memoryPtr->set(REGISTER_ADDR_LY, 0, false);
    }

    if (lyRegValue < 144) // Not V-BLANK
    {
        const TileDataSelector tileDataSelector{(lcdcRegValue & 0b00010000) ? TileDataSelector::Lower : TileDataSelector::Higher};
        const uint16_t bgTileMapStart{(lcdcRegValue & 0b00001000) ? (uint16_t)TILE_MAP_H_START : (uint16_t)TILE_MAP_L_START};

        /*
        const TileDataSelector tileDataSelector{TileDataSelector::Lower};
        const int bgTileMapStart{TILE_MAP_L_START};
         */

        // Index of tile in current row
        for (int rowTileI{}; rowTileI < TILE_MAP_TILES_PER_ROW; ++rowTileI)
        {
            // Index of pixel in the current row of the current tile
            for (int tileRowPixelI{}; tileRowPixelI < TILE_SIZE; ++tileRowPixelI)
            {
                uint8_t r, g, b;

                //const uint8_t colorI{getPixelColorIndex(lyRegValue/8*TILE_MAP_TILES_PER_ROW+rowTileI, lyRegValue%TILE_MAP_TILES_PER_ROW*8+tileRowPixelI%8, tileDataSelector)};
                const uint8_t colorI{getPixelColorIndex(
                        m_memoryPtr->get(bgTileMapStart+lyRegValue/TILE_SIZE*TILE_MAP_TILES_PER_ROW+rowTileI, false), // Tile index
                        lyRegValue%TILE_SIZE*TILE_SIZE+tileRowPixelI, // Pixel index
                        tileDataSelector)}; // Tile data selector
                //const uint8_t colorI{rowPixelI%4};

#ifdef PPU_IGNORE_BPR
                static constexpr uint8_t shades[]{
                    255, // 0 - white
                    200, // 1 - light gray
                    100, // 2 - dark gray
                    0    // 3 - black
                };

                r = shades[colorI]; g = shades[colorI]; b = shades[colorI];
#else
                getColorFromIndex(colorI, &r, &g, &b);
#endif

                SDL_SetRenderDrawColor(m_rendererPtr, r, g, b, 255);

                uint8_t scrollX{m_memoryPtr->get(REGISTER_ADDR_SCX, false)};
                uint8_t scrollY{m_memoryPtr->get(REGISTER_ADDR_SCY, false)};
                SDL_Rect pixelRect{
                    //m_currentBgMapByteI%TILE_MAP_TILES_PER_ROW*TILE_SIZE*PIXEL_SCALE+pixelI%TILE_SIZE*PIXEL_SCALE,
                    //m_currentBgMapByteI/TILE_MAP_TILES_PER_ROW*TILE_SIZE*PIXEL_SCALE+pixelI/TILE_SIZE*PIXEL_SCALE,
                    (rowTileI*TILE_SIZE+tileRowPixelI-scrollX)*PIXEL_SCALE,
                    (lyRegValue-scrollY)*PIXEL_SCALE,
                    PIXEL_SCALE,
                    PIXEL_SCALE};
                if (pixelRect.x > -PIXEL_SCALE && pixelRect.x < TILE_MAP_DISPLAYED_TILES_PER_ROW*TILE_SIZE*PIXEL_SCALE &&
                    pixelRect.y > -PIXEL_SCALE && pixelRect.y < TILE_MAP_DISPLAYED_TILES_PER_COL*TILE_SIZE*PIXEL_SCALE)
                    SDL_RenderFillRect(m_rendererPtr, &pixelRect);

#ifdef PPU_DRAW_GRID
                SDL_SetRenderDrawColor(m_rendererPtr, 255, 0, 0, 255);
                SDL_RenderDrawRect(m_rendererPtr, &pixelRect);
#endif
            }
        }
    }

    m_memoryPtr->set(REGISTER_ADDR_LY, lyRegValue+1, false);
}
