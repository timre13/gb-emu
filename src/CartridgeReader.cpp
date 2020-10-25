#include "CartridgeReader.h"
#include "Logger.h"
#include <iomanip>
#include <iostream>
#include <cstring>
#include "string_formatting.h"

//#define CARTRIDGE_READER_NO_COPY_CHECK

CartridgeReader::CartridgeReader(const std::string &filename)
    : m_filename{filename}
{
    m_romFile.open(m_filename, std::ios::binary);

    if (m_romFile.fail() || !m_romFile.is_open())
        Logger::fatal("Failed to open ROM file: " + m_filename + "\nReason: " + std::strerror(errno));
    else
        Logger::info("Opened ROM file");

    initCartridgeInfo();
}

void CartridgeReader::initCartridgeInfo()
{
    if (!m_romFile.is_open())
        Logger::fatal("Cartridge is not opened!");

    Logger::info("Reading cartridge info");


    // Read the title
    m_romFile.seekg(0x0134, std::ios::beg);
    m_romFile.read(m_cartridgeInfo.title, 16);


    // Read the cartridge type
    m_romFile.seekg(0x0147, std::ios::beg);
    m_romFile.read(reinterpret_cast<char*>(&m_cartridgeInfo.MBCType), 1);


    // Read the ROM size
    m_romFile.seekg(0x0148, std::ios::beg);
    uint8_t romSizeCode{};
    m_romFile.read(reinterpret_cast<char*>(&romSizeCode), 1);
    Logger::info("ROM size code: " + toHexStr(romSizeCode));
    switch (romSizeCode)
    {
    case 0x52:  m_cartridgeInfo.romSize = 1153433;                 break;
    case 0x53:  m_cartridgeInfo.romSize = 1258291;                 break;
    case 0x54:  m_cartridgeInfo.romSize = 1572864;                 break;
    default:    m_cartridgeInfo.romSize = (32767 << romSizeCode);  break;
    }


    // Calculate the number of ROM banks
    switch (romSizeCode)
    {
    case 0x00: m_cartridgeInfo.romBanks =  0; break;
    case 0x52: m_cartridgeInfo.romBanks = 72; break;
    case 0x53: m_cartridgeInfo.romBanks = 80; break;
    case 0x54: m_cartridgeInfo.romBanks = 96; break;
    default: if (romSizeCode <= 0x08) m_cartridgeInfo.romBanks = (2 << romSizeCode);
             else Logger::fatal("Invalid ROM size code: " + toHexStr(romSizeCode));
        break;
    }


    // Read the RAM size
    m_romFile.seekg(0x0149, std::ios::beg);
    uint8_t ramSizeCode{};
    m_romFile.read(reinterpret_cast<char*>(&ramSizeCode), 1);
    Logger::info("RAM size code: " +  toHexStr(ramSizeCode));
    switch (ramSizeCode)
    {
    case 0x00: m_cartridgeInfo.ramSize =      0; break;
    case 0x01: m_cartridgeInfo.ramSize =   2048; break;
    case 0x02: m_cartridgeInfo.ramSize =   8192; break;
    case 0x03: m_cartridgeInfo.ramSize =  32768; break;
    case 0x04: m_cartridgeInfo.ramSize = 131072; break;
    case 0x05: m_cartridgeInfo.ramSize =  65536; break;
    default: Logger::fatal("Invalid RAM size code: "+toHexStr(ramSizeCode)); break;
    }


    // Get the number of RAM banks
    switch (ramSizeCode)
    {
    case 0x00: m_cartridgeInfo.ramBanks =  0;  break;
    case 0x01: m_cartridgeInfo.ramBanks =  1;  break;
    case 0x02: m_cartridgeInfo.ramBanks =  1;  break;
    case 0x03: m_cartridgeInfo.ramBanks =  4;  break;
    case 0x04: m_cartridgeInfo.ramBanks = 16;  break;
    case 0x05: m_cartridgeInfo.ramBanks = 64;  break;
    default:                        break; // Already handled
    }


    // Get whether Super Game Boy is supported
    m_romFile.seekg(0x0146, std::ios::beg);
    uint8_t isSuperGameBoySupportedFlag{};
    m_romFile.read(reinterpret_cast<char*>(&isSuperGameBoySupportedFlag), 1);
    m_cartridgeInfo.isSGBSupported = (isSuperGameBoySupportedFlag == 0x03);


    // Get whether Game Boy Color is supported
    m_romFile.seekg(0x0143, std::ios::beg);
    uint8_t isCGBOnlyFlag{};
    m_romFile.read(reinterpret_cast<char*>(&isCGBOnlyFlag), 1);
    m_cartridgeInfo.isCGBOnly = (isCGBOnlyFlag == 0xc0);


    // Get destination
    m_romFile.seekg(0x014a, std::ios::beg);
    uint8_t isNonJapaneseFlag{};
    m_romFile.read(reinterpret_cast<char*>(&isNonJapaneseFlag), 1);
    m_cartridgeInfo.isJapanese = !isNonJapaneseFlag;


    // Get version of game
    m_romFile.seekg(0x014c, std::ios::beg);
    m_romFile.read(reinterpret_cast<char*>(&m_cartridgeInfo.gameVersion), 1);


    Logger::info("Cartridge info set");
}

void CartridgeReader::loadRomToMemory(Memory &memory, SDL_Renderer *renderer)
{
    Logger::info("Loading ROM to memory");

    Logger::info("ROM size: "+toHexStr(m_cartridgeInfo.romSize));

    m_romFile.seekg(0, std::ios::beg);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    uint8_t  currentByteValue{};
    uint32_t currentByteIndex{};
    while (!m_romFile.eof() && !m_romFile.fail() && currentByteIndex <= m_cartridgeInfo.romSize)
    {
        //Logger::info("Reading byte at: "+to_hex(currentByteIndex));

        m_romFile.read(reinterpret_cast<char*>(&currentByteValue), 1);

        memory.set(currentByteIndex, currentByteValue, false);

        ++currentByteIndex;
        m_romFile.seekg(currentByteIndex, std::ios::beg);


        if (currentByteIndex % (m_cartridgeInfo.romSize/100))
            continue;
        int progressbarWidth{static_cast<int>(std::round(static_cast<double>(currentByteIndex)/m_cartridgeInfo.romSize*WINDOW_WIDTH))};
        SDL_Rect progressbarRect{0, 0, progressbarWidth, WINDOW_HEIGHT};
        SDL_RenderFillRect(renderer, &progressbarRect);
        SDL_RenderPresent(renderer);
    }

    long readBytes{m_romFile.tellg()};

    Logger::info("Copied " + std::to_string(readBytes) + " bytes");

    memory.printRom0();

#ifndef CARTRIDGE_READER_NO_COPY_CHECK
    if (readBytes != currentByteIndex)
        Logger::fatal("Failed to copy ROM");
    else
        Logger::info("ROM copied");
#endif
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

CartridgeInfo CartridgeReader::getCartridgeInfo()
{
    return m_cartridgeInfo;
}

void CartridgeReader::closeRomFile()
{
    m_filename.clear();
    m_romFile.close();

    Logger::info("Closed ROM file");
}

CartridgeReader::~CartridgeReader()
{
}
