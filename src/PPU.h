#ifndef PPU_H
#define PPU_H

#include "config.h"

#include "common.h"

#include <SDL2/SDL.h>

#include "Memory.h"

#define PIXEL_SCALE 8
#define TILE_RAM_START 0x8000
#define TILE_SIZE 8
#define NUM_OF_TILES 384
#define PIXELS_PER_TILE TILE_SIZE*TILE_SIZE

#define BG_MAP_START 0x9800
#define BG_MAP_TILES_PER_ROW 14
#define BG_MAP_TILES_PER_COL 12

class PPU final
{
private:
    SDL_Renderer    *m_rendererPtr{nullptr};
    Memory          *m_memoryPtr{nullptr};

    int             m_currentBgMapByteI{};

public:
    enum class TileDataSelector
    {
        Lower,
        Higher,
    };

    PPU(SDL_Renderer *renderer, Memory *memory);

    uint8_t getPixelColorIndex(int tileI, int tilePixelI, TileDataSelector bgDataSelector) const;
    void getColorFromIndex(uint8_t index, uint8_t *rOut, uint8_t *gOut, uint8_t *bOut);

    void updateBackground();
};

#endif // PPU_H
