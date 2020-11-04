#include "Memory.h"

#include "Logger.h"
#include "common.h"
#include <iostream>
#include "string_formatting.h"

Memory::Memory(const CartridgeInfo *info)
{
    m_romBanks.resize(std::max(1, (int)info->romBanks));
    m_ramBanks.resize(std::max(1, (int)info->ramBanks));
}

uint8_t Memory::get(uint16_t address, bool log/*=true*/) const
{
    if (log) Logger::info("Memory read at address: "+toHexStr(address));

    if      (address <= 0x3fff) // ROM0 - Non-switchable ROM bank
        return m_rom0[address];
    else if (address <= 0x7fff) // ROMX - Switchable ROM bank
        return m_romBanks.at(m_currentRomBank).at(address-0x3fff-1);
    else if (address <= 0x9fff) // VRAM - Video RAM
        return m_vram.at(address-0x7fff-1);
    else if (address <= 0xbfff) // SRAM - External cartridge RAM
        return m_ramBanks.at(m_currentRamBank).at(address-0x9fff-1);
    else if (address <= 0xcfff) // WRAM0 - Work RAM
        return m_wram0.at(address-0xbfff-1);
    else if (address <= 0xdfff) // WRAMX - Work RAM
        return m_wram1.at(address-0xcfff-1);
    else if (address <= 0xfdff) // ECHO - Mirror RAM
        return get(address-0xbfff-1, log); // map the address to the start of WRAM0
    else if (address <= 0xfe9f) // OAM - Object Attribute RAM / Sprite information table
        return m_oam.at(address-0xfdff-1);
    else if (address <= 0xfeff) // UNUSED
        return 0;
    else if (address <= 0xff7f) // I/O Registers
        switch (address)
        {
        case REGISTER_ADDR_IF:
            return m_ifRegister | 0xf0; // The upper bits are always 1
        case REGISTER_ADDR_NR10:
            return m_nr10Register;
        case REGISTER_ADDR_NR11:
            return m_nr11Register;
        case REGISTER_ADDR_NR12:
            return m_nr12Register;
        case REGISTER_ADDR_NR13:
            return m_nr13Register;
        case REGISTER_ADDR_NR14:
            return m_nr14Register;
        case REGISTER_ADDR_NR21:
            return m_nr21Register;
        case REGISTER_ADDR_NR22:
            return m_nr22Register;
        case REGISTER_ADDR_NR23:
            return m_nr23Register;
        case REGISTER_ADDR_NR24:
            return m_nr24Register;
        case REGISTER_ADDR_NR30:
            return m_nr30Register;
        case REGISTER_ADDR_NR31:
            return m_nr31Register;
        case REGISTER_ADDR_NR32:
            return m_nr32Register;
        case REGISTER_ADDR_NR33:
            return m_nr33Register;
        case REGISTER_ADDR_NR34:
            return m_nr34Register;
        // TODO: Wave pattern RAM: 0xff30 - 0xff3f
        case REGISTER_ADDR_NR41:
            return m_nr41Register;
        case REGISTER_ADDR_NR42:
            return m_nr42Register;
        case REGISTER_ADDR_NR43:
            return m_nr43Register;
        case REGISTER_ADDR_NR44:
            return m_nr44Register;
        case REGISTER_ADDR_NR50:
            return m_nr50Register;
        case REGISTER_ADDR_NR51:
            return m_nr51Register;
        case REGISTER_ADDR_NR52:
            return m_nr52Register;
        case REGISTER_ADDR_LCDC:
            return m_lcdControlRegister;
        case REGISTER_ADDR_LCDSTAT:
            return m_lcdStatusRegister;
        case REGISTER_ADDR_SCY:
            return m_scyRegister;
        case REGISTER_ADDR_SCX:
            return m_scxRegister;
        case REGISTER_ADDR_LY:
            return m_lyRegister;
        case REGISTER_ADDR_LYC:
            return m_lycRegister;
        case REGISTER_ADDR_WY:
            return m_wyRegister;
        case REGISTER_ADDR_WX:
            return m_wxRegister;
        case REGISTER_ADDR_BGP:
            return m_bgpRegister;
        case REGISTER_ADDR_OBP0:
            return m_obp0Register;
        case REGISTER_ADDR_OBP1:
            return m_obp1Register;
        case REGISTER_ADDR_DMA:
            return m_dmaRegister;
        case 0xff4f: // VRAM bank selector, but DMG does not have switchable VRAM, so this returns 0xff
        case 0xff51: // HDMA1 - GBC only - always reads 0xff
        case 0xff52: // HDMA2 - GBC only - always reads 0xff
        case 0xff53: // HDMA3 - GBC only - always reads 0xff
        case 0xff54: // HDMA4 - GBC only - always reads 0xff
        case 0xff55: // HDMA5 - GBC only - always reads 0xff
        case 0xff68: // BCPS  - GBC only - always reads 0xff
        case 0xff69: // BDPD  - GBC only - always reads 0xff
        case 0xff6a: // OCPS  - GBC only - always reads 0xff
        case 0xff6b: // OCPD  - GBC only - always reads 0xff
        case 0xff70: // WRAM bank selector, but DMG does not have switchable WRAM, so this returns 0xff
            return 0xff;
        default:
            UNIMPLEMENTED(); // TODO: And the rest I/O registers?
            return 0;
        }
    else if (address <= 0xfffe) // HRAM - High RAM / internal CPU RAM
        return m_hram.at(address-0xff7f-1);
    else if (address == REGISTER_ADDR_IE) // IE Registers - Interrupt enable flags
        return m_ie; // the IE register is only one byte
    else
    {
        IMPOSSIBLE();
        return 0;
    }
}

void Memory::set(uint16_t address, uint8_t value, bool log/*=true*/)
{
    if (log) Logger::info("Memory written to address: "+toHexStr(address)+" with value: "+toHexStr(value));

    if      (address <= 0x3fff) // ROM0 - Non-switchable ROM bank
        m_rom0.at(address) = value;
    else if (address <= 0x7fff) // ROMX - Switchable ROM bank
        m_romBanks.at(m_currentRomBank).at(address-0x3fff-1) = value;
    else if (address <= 0x9fff) // VRAM - Video RAM
        m_vram.at(address-0x7fff-1) = value;
    else if (address <= 0xbfff) // SRAM - External cartridge RAM
        m_ramBanks.at(m_currentRamBank).at(address-0x9fff-1) = value;
    else if (address <= 0xcfff) // WRAM0 - Work RAM
        m_wram0.at(address-0xbfff-1) = value;
    else if (address <= 0xdfff) // WRAMX - Work RAM
        m_wram1.at(address-0xcfff-1) = value;
    else if (address <= 0xfdff) // ECHO - Mirror RAM
        set(address-0xbfff-1, value); // map the address to the start of WRAM0
    else if (address <= 0xfe9f) // OAM - Object Attribute Ram / Sprite information table
        m_oam.at(address-0xfdff-1) = value;
    else if (address <= 0xfeff) // UNUSED
    {
        // Writes are ignored
        (void)address;
        (void)value;
    }
    else if (address <= 0xff7f) // I/O Registers
        switch (address)
        {
        case REGISTER_ADDR_IF:
            m_ifRegister = value | 0xf0;
            break;
        case REGISTER_ADDR_NR10:
            m_nr10Register = value;
            break;
        case REGISTER_ADDR_NR11:
            m_nr11Register = value;
            break;
        case REGISTER_ADDR_NR12:
            m_nr12Register = value;
            break;
        case REGISTER_ADDR_NR13:
            m_nr13Register = value;
            break;
        case REGISTER_ADDR_NR14:
            m_nr14Register = value;
            break;
        case REGISTER_ADDR_NR21:
            m_nr21Register = value;
            break;
        case REGISTER_ADDR_NR22:
            m_nr22Register = value;
            break;
        case REGISTER_ADDR_NR23:
            m_nr23Register = value;
            break;
        case REGISTER_ADDR_NR24:
            m_nr24Register = value;
            break;
        case REGISTER_ADDR_NR30:
            m_nr30Register = value;
            break;
        case REGISTER_ADDR_NR31:
            m_nr31Register = value;
            break;
        case REGISTER_ADDR_NR32:
            m_nr32Register = value;
            break;
        case REGISTER_ADDR_NR33:
            m_nr33Register = value;
            break;
        case REGISTER_ADDR_NR34:
            m_nr34Register = value;
            break;
        // TODO: Wave pattern RAM: 0xff30 - 0xff3f
        case REGISTER_ADDR_NR41:
            break;
            m_nr41Register = value;
            break;
        case REGISTER_ADDR_NR42:
            m_nr42Register = value;
            break;
        case REGISTER_ADDR_NR43:
            m_nr43Register = value;
            break;
        case REGISTER_ADDR_NR44:
            m_nr44Register = value;
            break;
        case REGISTER_ADDR_NR50:
            m_nr50Register = value;
            break;
        case REGISTER_ADDR_NR51:
            m_nr51Register = value;
            break;
        case REGISTER_ADDR_NR52:
            m_nr52Register = value;
            break;
        case REGISTER_ADDR_LCDC:
            m_lcdControlRegister = value;
            break;
        case REGISTER_ADDR_LCDSTAT:
            m_lcdStatusRegister = value;
            break;
        case REGISTER_ADDR_SCY:
            m_scyRegister = value;
            break;
        case REGISTER_ADDR_SCX:
            m_scxRegister = value;
            break;
        case REGISTER_ADDR_LY:
            m_lyRegister = value;
            break;
        case REGISTER_ADDR_LYC:
            m_lycRegister = value;
            break;
        case REGISTER_ADDR_WY:
            m_wyRegister = value;
            break;
        case REGISTER_ADDR_WX:
            m_wxRegister = value;
            break;
        case REGISTER_ADDR_BGP:
            m_bgpRegister = value;
            break;
        case REGISTER_ADDR_OBP0:
            m_obp0Register = value;
            break;
        case REGISTER_ADDR_OBP1:
            m_obp1Register = value;
            break;
        case REGISTER_ADDR_DMA:
            m_dmaRegister = value;
            break;
        case 0xff4f: // VRAM bank selector, but DMG does not have switchable VRAM, so writes are ignored
        case 0xff51: // HDMA1 - GBC only - writes are ignored
        case 0xff52: // HDMA2 - GBC only - writes are ignored
        case 0xff53: // HDMA3 - GBC only - writes are ignored
        case 0xff54: // HDMA4 - GBC only - writes are ignored
        case 0xff55: // HDMA5 - GBC only - writes are ignored
        case 0xff68: // BCPS  - GBC only - writes are ignored
        case 0xff69: // BDPD  - GBC only - writes are ignored
        case 0xff6a: // OCPS  - GBC only - writes are ignored
        case 0xff6b: // OCPD  - GBC only - writes are ignored
        case 0xff70: // WRAM bank selector, but DMG does not have switchable WRAM, so writes are ignored
            (void)value;
            break;
        default:
            UNIMPLEMENTED(); // TODO: And the rest I/O registers?
            break;
        }
    else if (address <= 0xfffe) // HRAM - High RAM / internal CPU RAM
        m_hram.at(address-0xff7f-1) = value;
    else if (address == REGISTER_ADDR_IE) // IE Register - Interrupt enable flags
        m_ie = value;
    else
        IMPOSSIBLE();
}

void Memory::printRom0() const
{
    int memorySize{0x3fff+1};
    for (int i{}; i < memorySize; ++i)
    {
        uint8_t byte = get(i, false);

        if (i == 0x0100 || i == 0x0100+1)
            std::cout << '$';
        else
            std::cout << ' ';

        std::cout << std::hex << std::setw(2) << std::setfill('0') << (byte & 0xffff);

        if (!((i+1)%32))
            std::cout << '\n';
    }
    std::cout << '\n';
}

void Memory::printWhole() const
{
    std::cout << "---------------- start of memory ----------------" << '\n';

    int memorySize{0xffff+1};
    for (int i{}; i < memorySize; ++i)
    {
        uint8_t byte = get(i, false);

        if (i == 0x0100 || i == 0x0100+1)
            std::cout << '$';
        else
            std::cout << ' ';

        std::cout << std::hex << std::setw(2) << std::setfill('0') << (byte & 0xffff);

        if (!((i+1)%32))
            std::cout << '\n';
    }
    std::cout << '\n';

    std::cout << "----------------- end of memory -----------------" << '\n';
}
