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
        case 0xff0f:
            return m_ifRegister | 0xf0; // The upper bits are always 1
        case 0xff10:
            return m_nr10Register;
        case 0xff11:
            return m_nr11Register;
        case 0xff12:
            return m_nr12Register;
        case 0xff13:
            return m_nr13Register;
        case 0xff14:
            return m_nr14Register;
        case 0xff16:
            return m_nr21Register;
        case 0xff17:
            return m_nr22Register;
        case 0xff18:
            return m_nr23Register;
        case 0xff19:
            return m_nr24Register;
        case 0xff1a:
            return m_nr30Register;
        case 0xff1b:
            return m_nr31Register;
        case 0xff1c:
            return m_nr32Register;
        case 0xff1d:
            return m_nr33Register;
        case 0xff1e:
            return m_nr34Register;
        // TODO: Wave pattern RAM: 0xff30 - 0xff3f
        case 0xff20:
            return m_nr41Register;
        case 0xff21:
            return m_nr42Register;
        case 0xff22:
            return m_nr43Register;
        case 0xff23:
            return m_nr44Register;
        case 0xff24:
            return m_nr50Register;
        case 0xff25:
            return m_nr51Register;
        case 0xff26:
            return m_nr52Register;
        case 0xff40:
            return m_lcdControlRegister;
        case 0xff41:
            return m_lcdStatusRegister;
        case 0xff42:
            return m_scyRegister;
        case 0xff43:
            return m_scxRegister;
        case 0xff44:
            return m_lyRegister;
        case 0xff45:
            return m_lycRegister;
        case 0xff4a:
            return m_wyRegister;
        case 0xff4b:
            return m_wxRegister;
        case 0xff47:
            return m_bgpRegister;
        case 0xff48:
            return m_obp0Register;
        case 0xff49:
            return m_obp1Register;
        case 0xff46:
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
    else if (address == 0xffff) // IE Registers - Interrupt enable flags
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
        case 0xff0f:
            m_ifRegister = value | 0xf0;
            break;
        case 0xff10:
            m_nr10Register = value;
            break;
        case 0xff11:
            m_nr11Register = value;
            break;
        case 0xff12:
            m_nr12Register = value;
            break;
        case 0xff13:
            m_nr13Register = value;
            break;
        case 0xff14:
            m_nr14Register = value;
            break;
        case 0xff16:
            m_nr21Register = value;
            break;
        case 0xff17:
            m_nr22Register = value;
            break;
        case 0xff18:
            m_nr23Register = value;
            break;
        case 0xff19:
            m_nr24Register = value;
            break;
        case 0xff1a:
            m_nr30Register = value;
            break;
        case 0xff1b:
            m_nr31Register = value;
            break;
        case 0xff1c:
            m_nr32Register = value;
            break;
        case 0xff1d:
            m_nr33Register = value;
            break;
        case 0xff1e:
            m_nr34Register = value;
            break;
        // TODO: Wave pattern RAM: 0xff30 - 0xff3f
        case 0xff20:
            m_nr41Register = value;
            break;
        case 0xff21:
            m_nr42Register = value;
            break;
        case 0xff22:
            m_nr43Register = value;
            break;
        case 0xff23:
            m_nr44Register = value;
            break;
        case 0xff24:
            m_nr50Register = value;
            break;
        case 0xff25:
            m_nr51Register = value;
            break;
        case 0xff26:
            m_nr52Register = value;
            break;
        case 0xff40:
            m_lcdControlRegister = value;
            break;
        case 0xff41:
            m_lcdStatusRegister = value;
            break;
        case 0xff42:
            m_scyRegister = value;
            break;
        case 0xff43:
            m_scxRegister = value;
            break;
        case 0xff44:
            m_lyRegister = value;
            break;
        case 0xff45:
            m_lycRegister = value;
            break;
        case 0xff4a:
            m_wyRegister = value;
            break;
        case 0xff4b:
            m_wxRegister = value;
            break;
        case 0xff47:
            m_bgpRegister = value;
            break;
        case 0xff48:
            m_obp0Register = value;
            break;
        case 0xff49:
            m_obp1Register = value;
            break;
        case 0xff46:
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
    else if (address == 0xffff) // IE Register - Interrupt enable flags
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
