#include "PPU.h"

#include "Logger.h"
#include "string_formatting.h"

#include <iostream>

#define PPU_TEX_PIX_FORM SDL_PIXELFORMAT_RGBA32

// Ignore Background Palette Register
//#define PPU_IGNORE_BPR

#define PPU_SCANLINE_TCYCLES (456)
// OAM Scan mode
#define PPU_MODE_2_TCYCLES (80)
// Drawing mode
// This can vary, but let's not care about that for now
#define PPU_MODE_3_TCYCLES (175)
// H-Blank mode
// This depends on the length of mode 3.
// Tries to padd the duration of the scanline to 456 T-Cycles
#define PPU_MODE_0_TCYCLES (PPU_SCANLINE_TCYCLES-PPU_MODE_3_TCYCLES)
// V-Blank mode
// There are 10 pseudo-scanlines at the end of a frame
#define PPU_MODE_1_TCYCLES (10*PPU_SCANLINE_TCYCLES)

PPU::PPU(SDL_Renderer *renderer, Memory *memory)
    :
    m_rendererPtr{renderer},
    m_memoryPtr{memory},
    m_texture{SDL_CreateTexture(
            m_rendererPtr,
            PPU_TEX_PIX_FORM,
            SDL_TEXTUREACCESS_STREAMING,
            TILE_MAP_DISPLAYED_TILES_PER_ROW*TILE_SIZE,
            TILE_MAP_DISPLAYED_TILES_PER_COL*TILE_SIZE
            )}
{
    if (!m_texture) Logger::fatal("Failed to create texture for PPU: " + std::string(SDL_GetError()));
    SDL_LockTexture(m_texture, nullptr, (void**)&m_texDataPtr, &m_texPitch);
    m_texForm = SDL_AllocFormat(PPU_TEX_PIX_FORM);
    // TODO: Deallocate
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

uint8_t PPU::getPixelColorIndexFlat(uint tileI, int tilePixelI) const
{
    const uint16_t pixelDataAddress{(uint16_t)(TILE_DATA_UNSIGNED_START+tileI*TILE_SIZE*2+tilePixelI/TILE_SIZE*2)};
    uint8_t colorI{
            (uint8_t)
            (((m_memoryPtr->get(pixelDataAddress+0, false) & (1 << (TILE_SIZE-tilePixelI%TILE_SIZE-1))) ? 2 : 0) |
             ((m_memoryPtr->get(pixelDataAddress+1, false) & (1 << (TILE_SIZE-tilePixelI%TILE_SIZE-1))) ? 1 : 0))};

    return colorI;
}

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

    /*
    static constexpr SDL_Color palette[]{
            SDL_Color{255, 255, 255, 0xff},
            SDL_Color{200, 200, 200, 0xff},
            SDL_Color{100, 100, 100, 0xff},
            SDL_Color{  0,   0,   0, 0xff},
    };
    */

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

    /*
     * Request STAT interrupt.
     * This sets the STAT bit in IF.
     */
    auto reqStatInterrupt = [this](){
        const uint8_t ifVal = m_memoryPtr->get(REGISTER_ADDR_IF, false);
        m_memoryPtr->set(REGISTER_ADDR_IF, ifVal | INTERRUPT_MASK_LCDCSTAT, false);
    };

    if (lyRegValue < 144) // A normal scanline
    {
        const TileDataSelector tileDataSelector{(lcdcRegValue & LCDC_BIT_BG_WIN_TILE_DATA_AREA)
            ? TileDataSelector::Unsigned : TileDataSelector::Signed};
        const uint16_t bgTileMapStart{(lcdcRegValue & LCDC_BIT_BG_TILE_MAP_AREA) ? (uint16_t)TILE_MAP_H_START : (uint16_t)TILE_MAP_L_START};

        //if (tileDataSelector == TileDataSelector::Higher)
        //    Logger::warning("Unsigned tile data addressing is buggy (?)");

        // TODO: Signed?
        const uint8_t scrollX = /*(int8_t)*/m_memoryPtr->get(REGISTER_ADDR_SCX, false);
        const uint8_t scrollY = /*(int8_t)*/m_memoryPtr->get(REGISTER_ADDR_SCY, false);

        // Only render if visible (does this work well?)
        //if (lyRegValue > scrollY + TILE_MAP_DISPLAYED_TILES_PER_COL*TILE_SIZE)
        //    return;

        if (m_scanlineElapsed < PPU_MODE_2_TCYCLES) // The PPU is in mode 2
        {
            // Set the mode in STAT to 2
            m_memoryPtr->set(REGISTER_ADDR_LCDSTAT,
                    (m_memoryPtr->get(REGISTER_ADDR_LCDSTAT, false) & ~STAT_MASK_PPU_MODE) | STAT_PPU_MODE_2_VAL,
                    false);

            // If the mode 2 STAT interrupt is enabled, request it
            if (m_memoryPtr->get(REGISTER_ADDR_LCDSTAT, false) & STAT_BIT_MODE_2_INT_EN)
                reqStatInterrupt();

            // TODO: OAM scan, sprites are not supported yet
        }
        else if (m_scanlineElapsed < PPU_MODE_2_TCYCLES+PPU_MODE_3_TCYCLES) // The PPU is in mode 3
        {
            // Set the mode in STAT to 3
            m_memoryPtr->set(REGISTER_ADDR_LCDSTAT,
                    (m_memoryPtr->get(REGISTER_ADDR_LCDSTAT, false) & ~STAT_MASK_PPU_MODE) | STAT_PPU_MODE_3_VAL,
                    false);

            for (int i{}; i < 8 && m_xPos < 160; ++i)
            {
                const int screenx = m_xPos-scrollX;
                const int screeny = lyRegValue-scrollY;

                if (screenx >= 0 && screenx < 160 && screeny >= 0 && screeny < 144) // Only render if visible
                {
                    const uint8_t tileI = m_memoryPtr->get(
                            bgTileMapStart+lyRegValue/TILE_SIZE*TILE_MAP_TILES_PER_ROW+m_xPos/8,
                            false);
                    const uint8_t colorI = getPixelColorIndex(tileI, m_xPos%8+lyRegValue%8*8, tileDataSelector);
                    const SDL_Color color = mapIndexToColor(colorI);
                    assert(m_texDataPtr);
                    
                    const Uint32 mappedColor = SDL_MapRGBA(m_texForm, color.r, color.g, color.b, 255);
                    m_texDataPtr[screeny*m_texPitch/m_texForm->BytesPerPixel+screenx] = mappedColor;
                }

                ++m_xPos;
            }
        }
        else if (m_scanlineElapsed
                < PPU_MODE_2_TCYCLES+PPU_MODE_3_TCYCLES+PPU_MODE_0_TCYCLES) // The PPU is in mode 0
        {
            // Set the mode in STAT to 0
            m_memoryPtr->set(REGISTER_ADDR_LCDSTAT,
                    (m_memoryPtr->get(REGISTER_ADDR_LCDSTAT, false) & ~STAT_MASK_PPU_MODE) | STAT_PPU_MODE_0_VAL,
                    false);

            // If the mode 0 STAT interrupt is enabled, request it
            if (m_memoryPtr->get(REGISTER_ADDR_LCDSTAT, false) & STAT_BIT_MODE_0_INT_EN)
                reqStatInterrupt();

            // Do nothing
        }
    }
    else if (lyRegValue == 144 && m_scanlineElapsed == 0) // First scanline of of V-BLANK
    {
        // Set the mode in STAT to 1
        m_memoryPtr->set(REGISTER_ADDR_LCDSTAT,
                (m_memoryPtr->get(REGISTER_ADDR_LCDSTAT, false) & ~STAT_MASK_PPU_MODE) | STAT_PPU_MODE_1_VAL, false);

        // Call the V-blank interrupt
        m_memoryPtr->set(REGISTER_ADDR_IF, m_memoryPtr->get(REGISTER_ADDR_IF, false) | INTERRUPT_MASK_VBLANK, false);

        // If the mode 1 STAT interrupt is enabled, request it
        if (m_memoryPtr->get(REGISTER_ADDR_LCDSTAT, false) & STAT_BIT_MODE_1_INT_EN)
            reqStatInterrupt();

        SDL_UnlockTexture(m_texture);
        SDL_RenderCopy(m_rendererPtr, m_texture, nullptr, nullptr);
        SDL_LockTexture(m_texture, nullptr, (void**)&m_texDataPtr, &m_texPitch);
    }
    else if (lyRegValue > 153 && m_scanlineElapsed == 0) // End of V-BLANK
    {
        m_memoryPtr->set(REGISTER_ADDR_LY, 0, false);
        //Logger::info("V-Blank");
    }

    {
        const uint8_t lycVal = m_memoryPtr->get(REGISTER_ADDR_LYC, false);
        const uint8_t statVal = m_memoryPtr->get(REGISTER_ADDR_LCDSTAT, false);
        if (lycVal == lyRegValue)
        {
            // Set the coincidence flag
            m_memoryPtr->set(REGISTER_ADDR_LCDSTAT, statVal | STAT_BIT_COINCIDENCE, false);
            if (statVal & STAT_BIT_LYC_EQ_LY_INT_EN) // If the LYC=LY STAT interrupt is enabled
            {
                reqStatInterrupt();
            }
        }
        else
        {
            // Unset the coincidence flag
            m_memoryPtr->set(REGISTER_ADDR_LCDSTAT, statVal & ~STAT_BIT_COINCIDENCE, false);
        }
    }

#if 0 // TESTING
    m_memoryPtr->set(REGISTER_ADDR_LCDSTAT, m_memoryPtr->get(REGISTER_ADDR_LCDSTAT, false) & ~2, false);
#endif


    ++m_scanlineElapsed;
    if (m_scanlineElapsed == PPU_SCANLINE_TCYCLES) // If this is the end of a scanline
    {
        m_scanlineElapsed = 0;
        m_memoryPtr->set(REGISTER_ADDR_LY, lyRegValue+1, false);
        m_xPos = 0;
    }
}
