#include "Memory.h"

#include "Logger.h"
#include "common.h"
#include <iostream>
#include "string_formatting.h"

#define JOYP_BIT_SELECT_ACT_BTNS (1 << 5)
#define JOYP_BIT_SELECT_DIR_BTNS (1 << 4)
#define JOYP_BIT_DOWN_OR_START   (1 << 3)
#define JOYP_BIT_UP_OR_SELECT    (1 << 2)
#define JOYP_BIT_LEFT_OR_BTN_B   (1 << 1)
#define JOYP_BIT_RIGHT_OR_BTN_A  (1 << 0)
#define JOYP_MASK_ALL_BTNS       0x0f

Memory::Memory(const CartridgeInfo *info, SerialViewer *serial, Joypad *joypad, Timer *timer)
    : m_serial{serial}, m_joypadPtr{joypad}, m_timerPtr{timer}
{
    m_romBanks.resize(std::max(1, (int)info->romBanks));
    m_ramBanks.resize(std::max(1, (int)info->ramBanks));
}

uint8_t Memory::get(uint16_t address, bool log/*=true*/)
{
    //if (log) Logger::info("Memory read at address: "+toHexStr(address));

    if (log && m_dmaRemainingCycles > 0)
    {
        // While DMA is active, only the HRAM is usable
        // Other areas return 0xff
        if (address >= 0xff80 && address <= 0xfffe)
            return m_hram[address-0xff7f-1];
        else
            return 0xff;
    }


    if      (address <= 0x3fff) // ROM0 - Non-switchable ROM bank
        return m_rom0[address];
    else if (address <= 0x7fff) // ROMX - Switchable ROM bank
        return m_romBanks.at(m_currentRomBank)[address-0x3fff-1];
    else if (address <= 0x9fff) // VRAM - Video RAM
        return m_vram[address-0x7fff-1];
    else if (address <= 0xbfff) // SRAM - External cartridge RAM
        return m_ramBanks.at(m_currentRamBank)[address-0x9fff-1];
    else if (address <= 0xcfff) // WRAM0 - Work RAM
        return m_wram0[address-0xbfff-1];
    else if (address <= 0xdfff) // WRAMX - Work RAM
        return m_wram1[address-0xcfff-1];
    else if (address <= 0xfdff) // ECHO - Mirror RAM
        return get(address-0xbfff-1, log); // map the address to the start of WRAM0
    else if (address <= 0xfe9f) // OAM - Object Attribute RAM / Sprite information table
        return m_oam[address-0xfdff-1];
    else if (address <= 0xfeff) // UNUSED
        return 0;
    else if (address <= 0xff7f) // I/O Registers
        switch (address)
        {
        case REGISTER_ADDR_JOYP:
        {
            if ((m_joypRegister & JOYP_BIT_SELECT_ACT_BTNS) == 0) // If button (action) keys are selected
            {
                if (m_joypadPtr->isButtonPressed(Joypad::Button::Start))  m_joypRegister &= ~JOYP_BIT_DOWN_OR_START;
                if (m_joypadPtr->isButtonPressed(Joypad::Button::Select)) m_joypRegister &= ~JOYP_BIT_UP_OR_SELECT;
                if (m_joypadPtr->isButtonPressed(Joypad::Button::B))      m_joypRegister &= ~JOYP_BIT_LEFT_OR_BTN_B;
                if (m_joypadPtr->isButtonPressed(Joypad::Button::A))      m_joypRegister &= ~JOYP_BIT_RIGHT_OR_BTN_A;
            }
            if ((m_joypRegister & JOYP_BIT_SELECT_DIR_BTNS) == 0) // If direction keys are selected
            {
                if (m_joypadPtr->isButtonPressed(Joypad::Button::Down))  m_joypRegister &= ~JOYP_BIT_DOWN_OR_START;
                if (m_joypadPtr->isButtonPressed(Joypad::Button::Up))    m_joypRegister &= ~JOYP_BIT_UP_OR_SELECT;
                if (m_joypadPtr->isButtonPressed(Joypad::Button::Left))  m_joypRegister &= ~JOYP_BIT_LEFT_OR_BTN_B;
                if (m_joypadPtr->isButtonPressed(Joypad::Button::Right)) m_joypRegister &= ~JOYP_BIT_RIGHT_OR_BTN_A;
            }
            return m_joypRegister;
        }
        case REGISTER_ADDR_SB:
            return m_sb;
        case REGISTER_ADDR_SC:
            return 0; // TODO: What should we return?
        case REGISTER_ADDR_DIV:
            return m_timerPtr->getDivRegister();
        case REGISTER_ADDR_TIMA:
            return m_timerPtr->getTimaRegister();
        case REGISTER_ADDR_TMA:
            return m_timerPtr->getTmaRegister();
        case REGISTER_ADDR_TAC:
            return m_timerPtr->getTacRegister();
        case REGISTER_ADDR_IF:
            return m_ifRegister | 0b11100000; // The upper 3 bits are always 1
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
        case 0xff30:
        case 0xff31:
        case 0xff32:
        case 0xff33:
        case 0xff34:
        case 0xff35:
        case 0xff36:
        case 0xff37:
        case 0xff38:
        case 0xff39:
        case 0xff3a:
        case 0xff3b:
        case 0xff3c:
        case 0xff3d:
        case 0xff3e:
        case 0xff3f:
            // TODO: Wave pattern RAM: 0xff30 - 0xff3f
            return 0;
        case REGISTER_ADDR_LCDC:
            return m_lcdControlRegister;
        case REGISTER_ADDR_LCDSTAT:
            return m_lcdStatusRegister | (1 << 7);
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
        case 0xff7f: // Undocumented
            return 0xff;
        default:
            Logger::error("Unimplemented I/O register: " +toHexStr(address));
            UNIMPLEMENTED(); // TODO: And the rest I/O registers?
            return 0;
        }
    else if (address <= 0xfffe) // HRAM - High RAM / internal CPU RAM
        return m_hram[address-0xff7f-1];
    else if (address == REGISTER_ADDR_IE) // IE Registers - Interrupt enable flags
        return m_ie; // the IE register is only one byte
    else
    {
        IMPOSSIBLE();
        return 0;
    }

    Logger::error("Read memory address: "+toHexStr(address));
    IMPOSSIBLE();
    return 0;
}

void Memory::set(uint16_t address, uint8_t value, bool log/*=true*/)
{
    //if (log) Logger::info("Memory written to address: "+toHexStr(address)+" with value: "+toHexStr(value));
    
    if (log && m_dmaRemainingCycles > 0)
    {
        // While DMA is active, only the HRAM is usable
        // Writing to other areas is ignored
        if (address >= 0xff80 && address <= 0xfffe)
            m_hram[address-0xff7f-1] = value;
        return;
    }

    /*
    TODO: Implement this thing
    // If the LCD and PPU is disabled
    if ((lcdcRegValue & 0b10000000) == 0)
    {
        // If they were disabled outside a V-BLANK
        if (lyRegValue < 144)
            SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_WARNING,
                "Emulation Warning",
                "LDC disabled outside V-BLANK! This would damage a real hardware.",
                nullptr);

        return;
    }
    */

    if      (address <= 0x3fff) // ROM0 - Non-switchable ROM bank
        m_rom0[address] = value;
    else if (address <= 0x7fff) // ROMX - Switchable ROM bank
        m_romBanks.at(m_currentRomBank)[address-0x3fff-1] = value;
    else if (address <= 0x9fff) // VRAM - Video RAM
        m_vram[address-0x7fff-1] = value;
    else if (address <= 0xbfff) // SRAM - External cartridge RAM
        m_ramBanks.at(m_currentRamBank)[address-0x9fff-1] = value;
    else if (address <= 0xcfff) // WRAM0 - Work RAM
        m_wram0[address-0xbfff-1] = value;
    else if (address <= 0xdfff) // WRAMX - Work RAM
        m_wram1[address-0xcfff-1] = value;
    else if (address <= 0xfdff) // ECHO - Mirror RAM
        set(address-0xbfff-1, value); // map the address to the start of WRAM0
    else if (address <= 0xfe9f) // OAM - Object Attribute Ram / Sprite information table
        m_oam[address-0xfdff-1] = value;
    else if (address <= 0xfeff) // UNUSED
    {
        // Writes are ignored
        (void)address;
        (void)value;
    }
    else if (address <= 0xff7f) // I/O Registers
        switch (address)
        {
        case REGISTER_ADDR_JOYP:
            // Ignore the non-selector bits
            m_joypRegister = value | 0b11001111;
            break;
        case REGISTER_ADDR_SB:
            m_sb = value;
            break;
        case REGISTER_ADDR_SC:
            // TODO: The other bits?
            if (value & 0b10000000) // If bit 7 is set
            {
                m_serial->write(m_sb);  // Write the data in SB to the serial port
                value &= 0b01111111; // Unset bit 7
                m_ifRegister |= INTERRUPT_MASK_SERIAL; // Call the serial interrupt
            }
            break;
        case REGISTER_ADDR_DIV:
            // Writing anything to DIV resets it
            m_timerPtr->resetDivRegister();
            break;
        case REGISTER_ADDR_TIMA:
            m_timerPtr->setTimaRegister(value);
            break;
        case REGISTER_ADDR_TMA:
            m_timerPtr->setTmaRegister(value);
            break;
        case REGISTER_ADDR_TAC:
            m_timerPtr->setTacRegister(value);
            break;
        case REGISTER_ADDR_IF:
            m_ifRegister = value;
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
        case 0xff30:
        case 0xff31:
        case 0xff32:
        case 0xff33:
        case 0xff34:
        case 0xff35:
        case 0xff36:
        case 0xff37:
        case 0xff38:
        case 0xff39:
        case 0xff3a:
        case 0xff3b:
        case 0xff3c:
        case 0xff3d:
        case 0xff3e:
        case 0xff3f:
            // TODO: Wave pattern RAM: 0xff30 - 0xff3f
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
        {
            m_dmaRemainingCycles = 160;
            m_dmaRegister = value;
            Logger::info("Starting DMA: "+toHexStr(value));
            assert(value <= 0xdf);
            const uint16_t source = (uint16_t(value) << 8);
            for (int i{}; i < 160; ++i)
            {
                m_oam[i] = get(source+i, false);
                //set(0xfe00+i, get(source+i, false), false);
                //Logger::info("DMA transfer from "+toHexStr(source+i)+" to "+toHexStr(0xfe00+i));
            }
        }
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
        case 0xff7f: // Undocumented
            (void)value;
            break;
        default:
            Logger::error("Unimplemented I/O register: " + toHexStr(address));
            UNIMPLEMENTED(); // TODO: And the rest I/O registers?
            break;
        }
    else if (address <= 0xfffe) // HRAM - High RAM / internal CPU RAM
        m_hram[address-0xff7f-1] = value;
    else if (address == REGISTER_ADDR_IE) // IE Register - Interrupt enable flags
        m_ie = value;
    else
        IMPOSSIBLE();
}

void Memory::printRom0()
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

void Memory::printWhole()
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
