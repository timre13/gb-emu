#ifndef MEMORY_H_
#define MEMORY_H_

#include "config.h"

struct CartridgeInfo;

#include "common.h"
#include "CartridgeReader.h"
#include "SerialViewer.h"

#include <stdint.h>
#include <vector>
#include <array>

// Addresses of memory-mapped registers
#define REGISTER_ADDR_SB      0xff01
#define REGISTER_ADDR_SC      0xff02
#define REGISTER_ADDR_IF      0xff0f
#define REGISTER_ADDR_NR10    0xff10
#define REGISTER_ADDR_NR11    0xff11
#define REGISTER_ADDR_NR12    0xff12
#define REGISTER_ADDR_NR13    0xff13
#define REGISTER_ADDR_NR14    0xff14
#define REGISTER_ADDR_NR21    0xff16
#define REGISTER_ADDR_NR22    0xff17
#define REGISTER_ADDR_NR23    0xff18
#define REGISTER_ADDR_NR24    0xff19
#define REGISTER_ADDR_NR30    0xff1a
#define REGISTER_ADDR_NR31    0xff1b
#define REGISTER_ADDR_NR32    0xff1c
#define REGISTER_ADDR_NR33    0xff1d
#define REGISTER_ADDR_NR34    0xff1e
#define WAVE_PATTER_RAM_START 0xff30
#define WAVE_PATTER_RAM_END   0xff3f
#define REGISTER_ADDR_NR41    0xff20
#define REGISTER_ADDR_NR42    0xff21
#define REGISTER_ADDR_NR43    0xff22
#define REGISTER_ADDR_NR44    0xff23
#define REGISTER_ADDR_NR50    0xff24
#define REGISTER_ADDR_NR51    0xff25
#define REGISTER_ADDR_NR52    0xff26
#define REGISTER_ADDR_LCDC    0xff40
#define REGISTER_ADDR_LCDSTAT 0xff41
#define REGISTER_ADDR_SCY     0xff42
#define REGISTER_ADDR_SCX     0xff43
#define REGISTER_ADDR_LY      0xff44
#define REGISTER_ADDR_LYC     0xff45
#define REGISTER_ADDR_WY      0xff4a
#define REGISTER_ADDR_WX      0xff4b
#define REGISTER_ADDR_BGP     0xff47
#define REGISTER_ADDR_OBP0    0xff48
#define REGISTER_ADDR_OBP1    0xff49
#define REGISTER_ADDR_DMA     0xff46
#define REGISTER_ADDR_IE      0xffff

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
    /// - tile RAM (data about graphics) and
    // background RAM (where these should be placed) are here
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

    // OAM table - sprite data is here
    std::array<uint8_t, 0x9f + 1>                   m_oam{};

    // Not usable
    // We just always read 0 here and ignore writes.

    // Memory-mapped registers
    // SB (Serial transfer data) - 0xff01
    uint8_t                                         m_sb{};
    // SC (Serial transfer control) - 0xff02
    // ---
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
    // LCDC - 0xff40
    uint8_t                                         m_lcdControlRegister{0b10010001}; // 0x91
    // LCD STAT - 0xff41
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

    // -------------------------------------------------------------------------

    SerialViewer                                    *m_serial{nullptr};

public:
    Memory(const CartridgeInfo *info, SerialViewer *serial);

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
    inline uint32_t getOpcodeNoSwap(uint16_t address) const
    {
        return
            (get(address+0, false) << 24) |
            (get(address+1, false) << 16) |
            (get(address+2, false) <<  8);
    }

    void printRom0() const;
    void printWhole() const;
};


#endif /* MEMORY_H_ */
