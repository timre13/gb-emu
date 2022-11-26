#include "PPU.h"

#include "Logger.h"
#include "string_formatting.h"

#include <iostream>

// Ignore Background Palette Register
//#define PPU_IGNORE_BPR

PPU::PPU(SDL_Renderer *renderer, Memory *memory)
    :
    m_rendererPtr{renderer},
    m_memoryPtr{memory},
    m_texture{SDL_CreateTexture(
            m_rendererPtr,
            SDL_PIXELFORMAT_RGB888,
            SDL_TEXTUREACCESS_TARGET,
            TILE_MAP_TILES_PER_ROW*TILE_SIZE,
            TILE_MAP_TILES_PER_COL*TILE_SIZE
            )}
{
    if (!m_texture) Logger::fatal("Failed to create texture for PPU: " + std::string(SDL_GetError()));
}

uint8_t PPU::getPixelColorIndex(uint8_t tileI, int tilePixelI, TileDataSelector bgDataSelector) const
{
    const uint16_t dataAddrBase = uint16_t((bgDataSelector == TileDataSelector::Unsigned
                ? TILE_DATA_UNSIGNED_START : TILE_DATA_SIGNED_START));
    // This should be ok
    const int tileI_ = (bgDataSelector == TileDataSelector::Unsigned ? tileI : (int8_t)tileI);
    const uint16_t pixelDataAddress{(uint16_t)(dataAddrBase+tileI_*TILE_SIZE*2+tilePixelI/TILE_SIZE*2)};
    uint8_t colorI{
            (uint8_t)
            (((m_memoryPtr->get(pixelDataAddress+0, false) & (1 << (TILE_SIZE-tilePixelI%TILE_SIZE-1))) ? 2 : 0) |
             ((m_memoryPtr->get(pixelDataAddress+1, false) & (1 << (TILE_SIZE-tilePixelI%TILE_SIZE-1))) ? 1 : 0))};

    return colorI;
}

void PPU::getColorFromIndex(uint8_t index, uint8_t *rOut, uint8_t *gOut, uint8_t *bOut)
SDL_Color PPU::mapIndexToColor(uint8_t index)
{
    // Get the value of the Background Palette Register
    const uint8_t bgpValue{m_memoryPtr->get(REGISTER_ADDR_BGP, false)};

    static constexpr SDL_Color palette[]{
            SDL_Color{0x82, 0x78, 0x0d, 0xff},
            SDL_Color{0x3a, 0x53, 0x36, 0xff},
            SDL_Color{0x5c, 0x71, 0x22, 0xff},
            SDL_Color{0x1c, 0x36, 0x28, 0xff},
    };

    // Get which color is mapped to the color index
    const int paletteEntryI{(bgpValue & (3 << index*2)) >> index*2};

    return palette[paletteEntryI];
}

void PPU::updateBackground()
{
    const uint8_t lcdcRegValue{m_memoryPtr->get(REGISTER_ADDR_LCDC, false)};

    // If the LCD and PPU are disabled
    if ((lcdcRegValue & LCDC_BIT_LCD_PPU_ENABLE) == 0)
        return;

    const uint8_t lyRegValue{m_memoryPtr->get(REGISTER_ADDR_LY, false)};

    if (lyRegValue == 144) // Start of V-BLANK
    {
        // Call the V-blank interrupt
        m_memoryPtr->set(REGISTER_ADDR_IF, m_memoryPtr->get(REGISTER_ADDR_IF, false) | INTERRUPT_MASK_VBLANK, false);

        // The cleared color will be overwritten, but we should clear the
        // renderer before modifying it as the SDL2 documentation says.
        SDL_RenderClear(m_rendererPtr);

        // Copy the texture to the renderer
        SDL_Rect winRect{
                0, 0,
                TILE_MAP_TILES_PER_ROW*TILE_SIZE*PIXEL_SCALE, TILE_MAP_TILES_PER_COL*TILE_SIZE*PIXEL_SCALE};
        if (SDL_RenderCopy(m_rendererPtr, m_texture, nullptr, &winRect))
            Logger::fatal("Failed to copy PPU texture: " + std::string(SDL_GetError()));
    }

    if (lyRegValue > 153) // End of V-BLANK
    {
        m_memoryPtr->set(REGISTER_ADDR_LY, 0, false);
    }

    if (lyRegValue < 144) // Not V-BLANK
    {
        const TileDataSelector tileDataSelector{(lcdcRegValue & LCDC_BIT_BG_WIN_TILE_DATA_AREA)
            ? TileDataSelector::Unsigned : TileDataSelector::Signed};
        const uint16_t bgTileMapStart{(lcdcRegValue & LCDC_BIT_BG_TILE_MAP_AREA) ? (uint16_t)TILE_MAP_H_START : (uint16_t)TILE_MAP_L_START};

        //if (tileDataSelector == TileDataSelector::Higher)
        //    Logger::warning("Unsigned tile data addressing is buggy (?)");

        int8_t scrollX{(int8_t)m_memoryPtr->get(REGISTER_ADDR_SCX, false)};
        int8_t scrollY{(int8_t)m_memoryPtr->get(REGISTER_ADDR_SCY, false)};

        // Only render if visible (does this work well?)
        if (lyRegValue > scrollY + TILE_MAP_DISPLAYED_TILES_PER_COL*TILE_SIZE)
            return;

        SDL_SetRenderTarget(m_rendererPtr, m_texture);

        // Index of tile in current row (only render the visible part)
        for (int rowTileI{scrollX/TILE_SIZE}; rowTileI < TILE_MAP_DISPLAYED_TILES_PER_ROW+scrollX/TILE_SIZE; ++rowTileI)
        {
            // Index of pixel in the current row of the current tile
            for (int tileRowPixelI{}; tileRowPixelI < TILE_SIZE; ++tileRowPixelI)
            {
                int pixelX{rowTileI*TILE_SIZE+tileRowPixelI-scrollX};
                int pixelY{lyRegValue-scrollY};
                if (!(pixelX >= 0 && pixelX < TILE_MAP_DISPLAYED_TILES_PER_ROW*TILE_SIZE &&
                    pixelY >= 0 && pixelY < TILE_MAP_DISPLAYED_TILES_PER_COL*TILE_SIZE))
                        continue;

                const uint8_t colorI{getPixelColorIndex(
                        m_memoryPtr->get(bgTileMapStart+lyRegValue/TILE_SIZE*TILE_MAP_TILES_PER_ROW+rowTileI, false), // Tile index
                        lyRegValue%TILE_SIZE*TILE_SIZE+tileRowPixelI, // Pixel index
                        tileDataSelector)}; // Tile data selector

#ifdef PPU_IGNORE_BPR // FIXME
                static constexpr uint8_t shades[]{
                    255, // 0 - white
                    200, // 1 - light gray
                    100, // 2 - dark gray
                    0    // 3 - black
                };

                r = shades[colorI]; g = shades[colorI]; b = shades[colorI];
#else
                SDL_Color color = mapIndexToColor(colorI);
#endif
                SDL_SetRenderDrawColor(m_rendererPtr, color.r, color.g, color.b, color.a);
                SDL_RenderDrawPoint(m_rendererPtr, pixelX, pixelY);
            }
        }
    }

    SDL_SetRenderTarget(m_rendererPtr, nullptr);

    m_memoryPtr->set(REGISTER_ADDR_LY, lyRegValue+1, false);
}
