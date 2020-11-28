#ifndef PPU_H
#define PPU_H

#include "config.h"
#include "common.h"

#include "Memory.h"

#include <SDL2/SDL.h>

#define PIXEL_SCALE 7
#define TILE_DATA_L_START 0x8000
#define TILE_DATA_H_START 0x8800
#define TILE_SIZE 8
#define NUM_OF_TILES 384
#define PIXELS_PER_TILE TILE_SIZE*TILE_SIZE

#define TILE_MAP_L_START 0x9800
#define TILE_MAP_H_START 0x9c00
#define TILE_MAP_TILES_PER_ROW 64
#define TILE_MAP_TILES_PER_COL 64
#define TILE_MAP_DISPLAYED_TILES_PER_ROW 20
#define TILE_MAP_DISPLAYED_TILES_PER_COL 18

class PPU final
{
private:
    SDL_Renderer    *m_rendererPtr{nullptr};
    Memory          *m_memoryPtr{nullptr};

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
