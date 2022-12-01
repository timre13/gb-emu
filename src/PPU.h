#ifndef PPU_H
#define PPU_H

#include "config.h"
#include "common.h"

#include "Memory.h"

#include <SDL2/SDL.h>

#define PIXEL_SCALE 5
#define TILE_DATA_UNSIGNED_START 0x8000
#define TILE_DATA_SIGNED_START 0x9000
#define TILE_SIZE 8
#define NUM_OF_TILES (128*3)
#define PIXELS_PER_TILE TILE_SIZE*TILE_SIZE

#define TILE_MAP_L_START 0x9800
#define TILE_MAP_H_START 0x9c00
#define TILE_MAP_TILES_PER_ROW 32
#define TILE_MAP_TILES_PER_COL 64
#define TILE_MAP_DISPLAYED_TILES_PER_ROW 20
#define TILE_MAP_DISPLAYED_TILES_PER_COL 18

#define LCDC_BIT_BG_WIN_ENABLE         (1 << 0)
#define LCDC_BIT_OBJ_ENABLE            (1 << 1)
#define LCDC_BIT_OBJ_SIZE              (1 << 2)
#define LCDC_BIT_BG_TILE_MAP_AREA      (1 << 3)
#define LCDC_BIT_BG_WIN_TILE_DATA_AREA (1 << 4)
#define LCDC_BIT_WIN_ENABLE            (1 << 5)
#define LCDC_BIT_WIN_TILE_MAP_AREA     (1 << 6)
#define LCDC_BIT_LCD_PPU_ENABLE        (1 << 7)

class PPU final
{
private:
    SDL_Renderer    *m_rendererPtr{nullptr};
    Memory          *m_memoryPtr{nullptr};

    SDL_Texture     *m_texture{};

    int m_xPos{};
    int m_scanlineElapsed{};

public:
    enum class TileDataSelector
    {
        // 8000 method (base: 0x8000)
        Unsigned,
        // 8800 method (base: 0x9000)
        Signed,
    };

    PPU(SDL_Renderer *renderer, Memory *memory);

    uint8_t getPixelColorIndex(uint8_t tileI, int tilePixelI, TileDataSelector bgDataSelector) const;
    uint8_t getPixelColorIndexFlat(uint tileI, int tilePixelI) const;
    SDL_Color mapIndexToColor(uint8_t index);

    inline bool isScanlineStart() const { return m_scanlineElapsed == 0; }

    void updateBackground();
};

#endif // PPU_H
