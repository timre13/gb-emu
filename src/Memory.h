#ifndef MEMORY_H_
#define MEMORY_H_

#include "config.h"

struct CartridgeInfo;

#include "CartridgeReader.h"

#include <stdint.h>
#include <vector>
#include <array>

class Memory final
{
private:
    // Non-switchable ROM bank
    std::array<uint8_t, 0x3fff + 1>                 m_rom0{};

    // Switchable ROM banks
    std::vector<std::array<uint8_t, 0x4000 + 1>>    m_romBanks{};
    // Index of active ROM bank
    uint8_t                                         m_currentRomBank{};

    // Video RAM
    std::array<uint8_t, 0x1fff + 1>                 m_vram{};

    // External RAM banks
    std::vector<std::array<uint8_t, 0x1fff + 1>>    m_ramBanks{}; // ???
    // Index of current RAM bank
    uint8_t                                         m_currentRamBank{};

    // Work RAM
    std::array<uint8_t, 0xfff + 1>                  m_wram0{};

    // Work RAM bank, not switchable
    std::array<uint8_t, 0xfff + 1>                  m_wram1{};

    // ECHO RAM
    // Actually WRAM0 and WRAM1

    // OAM table
    std::array<uint8_t, 0x9f + 1>                   m_oam{};

    // Not usable
    // We just always read 0 here and ignore writes.

    // Memory-mapped registers
    // IF (Interrupt Flag) - 0xff0f
    uint8_t                                         m_ifRegister{0xf0};
    // NR10 - 0xff10
    uint8_t                                         m_nr10Register{};
    // NR11 - 0xff11
    uint8_t                                         m_nr11Register{};
    // NR12 - 0xff12
    uint8_t                                         m_nr12Register{};
    // NR13 - 0xff13
    uint8_t                                         m_nr13Register{};
    // NR14 - 0xff14
    uint8_t                                         m_nr14Register{};
    // NR21 - 0xff16
    uint8_t                                         m_nr21Register{};
    // NR22 - 0xff17
    uint8_t                                         m_nr22Register{};
    // NR23 - 0xff18
    uint8_t                                         m_nr23Register{};
    // NR24 - 0xff19
    uint8_t                                         m_nr24Register{};
    // NR30 - 0xff1a
    uint8_t                                         m_nr30Register{};
    // NR31 - 0xff1b
    uint8_t                                         m_nr31Register{};
    // NR32 - 0xff1c
    uint8_t                                         m_nr32Register{};
    // NR33 - 0xff1d
    uint8_t                                         m_nr33Register{};
    // NR34 - 0xff1e
    uint8_t                                         m_nr34Register{};
    // Wave Pattern RAM - 0xff30-0xff3f
    //std::array<uint8_t, 0x10>                       m_wavePatternRam{};
    // NR41 - 0xff20
    uint8_t                                         m_nr41Register{};
    // NR42 - 0xff21
    uint8_t                                         m_nr42Register{};
    // NR43 - 0xff22
    uint8_t                                         m_nr43Register{};
    // NR44 - 0xff23
    uint8_t                                         m_nr44Register{};
    // NR50 - 0xff24
    uint8_t                                         m_nr50Register{};
    // NR51 - 0xff25
    uint8_t                                         m_nr51Register{};
    // NR52 - 0xff26
    uint8_t                                         m_nr52Register{};
    // LCD  - 0xff40
    uint8_t                                         m_lcdControlRegister{};
    // LCD  - 0xff41
    uint8_t                                         m_lcdStatusRegister{};
    // SCY  - 0xff42
    uint8_t                                         m_scyRegister{};
    // SCX  - 0xff43
    uint8_t                                         m_scxRegister{};
    // LY   - 0xff44
    uint8_t                                         m_lyRegister{};
    // LYC  - 0xff45
    uint8_t                                         m_lycRegister{};
    // WY   - 0xff4a
    uint8_t                                         m_wyRegister{};
    // WX   - 0xff4b
    uint8_t                                         m_wxRegister{};
    // BGP  - 0xff47
    uint8_t                                         m_bgpRegister{};
    // OBP0 - 0xff48
    uint8_t                                         m_obp0Register{};
    // OBP1 - 0xff49
    uint8_t                                         m_obp1Register{};
    // DMA  - 0xff46
    uint8_t                                         m_dmaRegister{};

    // High RAM, actually in the CPU
    std::array<uint8_t, 0x7e + 1>                   m_hram{};

    // IE (Interrupt Enable Register) - 0xffff
    uint8_t                                         m_ie{};

public:
    Memory(const CartridgeInfo *info);

    uint8_t get(uint16_t address, bool log=true) const;
    void    set(uint16_t address, uint8_t value, bool log=true);

    inline uint16_t get16(uint16_t address, bool log=true) const
    {
        // Get 2 bytes and swap them (little endianess!)
        return get(address, log) | (get(address+1, log) << 8);
    }

    inline void set16(uint16_t address, uint16_t value)
    {
        // Set the 2 bytes swapped (little endianess!)
        set(address, (value&0x00ff));
        set(address+1, (value&0xff00)>>8);
    }

    // Gets a 24-bit value without endianess correction
    inline uint32_t getOpcodeNoSwap(uint16_t address, bool log=true) const
    {
        return
            (get(address+0, log) << 24) |
            (get(address+1, log) << 16) |
            (get(address+2, log) <<  8);
    }

    void printRom0() const;
    void printWhole() const;
};


#endif /* MEMORY_H_ */
