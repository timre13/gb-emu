#ifndef CARTRIDGEREADER_H_
#define CARTRIDGEREADER_H_

#include "Memory.h"

#include <string>
#include <fstream>
#include <stdint.h>
#include <SDL2/SDL.h>

extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;

class Memory;

struct CartridgeInfo
{
    char        title[16]{};
    uint8_t     MBCType{};
    uint32_t    romSize{};
    uint16_t    romBanks{};
    uint32_t    ramSize{};
    uint8_t     ramBanks{};
    bool        isCGBOnly{};
    bool        isSGBSupported{};
    bool        isJapanese{};
    uint8_t     gameVersion{};
};

class CartridgeReader final
{
private:
    std::string     m_filename;
    std::ifstream   m_romFile;
    CartridgeInfo   m_cartridgeInfo;

    void            initCartridgeInfo();

public:
    CartridgeReader(const std::string &filename);
    ~CartridgeReader();

    CartridgeInfo   getCartridgeInfo();
    void            loadRomToMemory(Memory &memory, SDL_Renderer *renderer);

    void            closeRomFile();
};

#endif /* CARTRIDGEREADER_H_ */
