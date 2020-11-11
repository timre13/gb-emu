#include "DebugWindow.h"
#include "Logger.h"
#include "string_formatting.h"
#include "opcode_names.h"

#include <bitset>
#include <stdint.h>

//#define DEBUG_TEXT_USE_COLORS

#define DEBUG_WINDOW_WIDTH  420
#define DEBUG_WINDOW_HEIGHT 920

DebugWindow::DebugWindow(int x, int y)
{
    m_x = x;
    m_y = y;
    m_w = DEBUG_WINDOW_WIDTH;
    m_h = DEBUG_WINDOW_HEIGHT;

    m_window = SDL_CreateWindow("Debugger", m_x, m_y, m_w, m_h, 0);

    if (!m_window)
        Logger::fatal("Failed to create debugger window");

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);

    if (!m_renderer)
        Logger::fatal("Failed to create renderer for debugger");

    m_fontManager = new FontManager{m_window};

    m_fontW = m_fontManager->get('A')->getW();
    m_fontH = m_fontManager->get('A')->getH();

    Logger::info("Debug window created");
}

void DebugWindow::renderText(const std::string &string, int x, int y, uint8_t colorR/*=0*/, uint8_t colorG/*=0*/, uint8_t colorB/*=0*/)
{
    for (size_t i{}; i < string.length(); ++i)
    {
        SDL_Texture *charTexture{m_fontManager->get(string[i])->getTexture()};
#ifdef DEBUG_TEXT_USE_COLORS
        SDL_SetTextureColorMod(charTexture, colorR, colorG, colorB);
#else
    (void) colorR;
    (void) colorG;
    (void) colorB;
#endif

        SDL_Rect destRect{x+(int)i*m_fontW, y, m_fontW, m_fontH};

        SDL_RenderCopy(m_renderer, charTexture, nullptr, &destRect);
    }
}

void DebugWindow::updateRegisterValues(const Registers *registers)
{
    renderText("===== 8-bit registers ====", 10, 10);
    auto regA{registers->getA()};
    renderText("A: "+toHexStr(regA, 2)+" | "+alignRight(std::to_string(regA), ' ', 3)+" | "+toBinStr(regA, 8), 10, 10+m_fontH*1);
    auto regB{registers->getB()};
    renderText("B: "+toHexStr(regB, 2)+" | "+alignRight(std::to_string(regB), ' ', 3)+" | "+toBinStr(regB, 8), 10, 10+m_fontH*2);
    auto regC{registers->getC()};
    renderText("C: "+toHexStr(regC, 2)+" | "+alignRight(std::to_string(regC), ' ', 3)+" | "+toBinStr(regC, 8), 10, 10+m_fontH*3);
    auto regD{registers->getD()};
    renderText("D: "+toHexStr(regD, 2)+" | "+alignRight(std::to_string(regD), ' ', 3)+" | "+toBinStr(regD, 8), 10, 10+m_fontH*4);
    auto regE{registers->getE()};
    renderText("E: "+toHexStr(regE, 2)+" | "+alignRight(std::to_string(regE), ' ', 3)+" | "+toBinStr(regE, 8), 10, 10+m_fontH*5);
    auto regF{registers->getF()};
    renderText("F: "+toHexStr(regF, 2)+" | "+alignRight(std::to_string(regF), ' ', 3)+" | "+toBinStr(regF, 8), 10, 10+m_fontH*6);
    auto regH{registers->getH()};
    renderText("H: "+toHexStr(regH, 2)+" | "+alignRight(std::to_string(regH), ' ', 3)+" | "+toBinStr(regH, 8), 10, 10+m_fontH*7);
    auto regL{registers->getL()};
    renderText("L: "+toHexStr(regL, 2)+" | "+alignRight(std::to_string(regL), ' ', 3)+" | "+toBinStr(regL, 8), 10, 10+m_fontH*8);
    renderText("==========================", 10, 10+m_fontH*9);

    renderText("=========== 16-bit registers ==========", 10, 10+m_fontH*11);
    auto regAF{registers->getAF()};
    renderText("AF: "+toHexStr(regAF, 4)+" | "+alignRight(std::to_string(regAF), ' ', 5)+" | "+toBinStr(regAF, 16), 10, 10+m_fontH*12);
    auto regBC{registers->getBC()};
    renderText("BC: "+toHexStr(regBC, 4)+" | "+alignRight(std::to_string(regBC), ' ', 5)+" | "+toBinStr(regBC, 16), 10, 10+m_fontH*13);
    auto regDE{registers->getDE()};
    renderText("DE: "+toHexStr(regDE, 4)+" | "+alignRight(std::to_string(regDE), ' ', 5)+" | "+toBinStr(regDE, 16), 10, 10+m_fontH*14);
    auto regHL{registers->getHL()};
    renderText("HL: "+toHexStr(regHL, 4)+" | "+alignRight(std::to_string(regHL), ' ', 5)+" | "+toBinStr(regHL, 16), 10, 10+m_fontH*15);
    auto regSP{registers->getSP()};
    renderText("SP: "+toHexStr(regSP, 4)+" | "+alignRight(std::to_string(regSP), ' ', 5)+" | "+toBinStr(regSP, 16), 10, 10+m_fontH*16);
    auto regPC{registers->getPC()};
    renderText("PC: "+toHexStr(regPC, 4)+" | "+alignRight(std::to_string(regPC), ' ', 5)+" | "+toBinStr(regPC, 16), 10, 10+m_fontH*17);
    renderText("=======================================", 10, 10+m_fontH*18);

    renderText("====== Flags ======", 10, 10+m_fontH*20);
    auto flagZ{registers->getZeroFlag()};
    renderText("Zero:       "+std::to_string(flagZ)+" | "+(flagZ ? "on" : "off"), 10, 10+m_fontH*21);
    auto flagN{registers->getNegativeFlag()};
    renderText("Negative:   "+std::to_string(flagN)+" | "+(flagN ? "on" : "off"), 10, 10+m_fontH*22);
    auto flagH{registers->getHalfCarryFlag()};
    renderText("Half Carry: "+std::to_string(flagH)+" | "+(flagH ? "on" : "off"), 10, 10+m_fontH*23);
    auto flagC{registers->getCarryFlag()};
    renderText("Carry:      "+std::to_string(flagC)+" | "+(flagC ? "on" : "off"), 10, 10+m_fontH*24);
    renderText("===================", 10, 10+m_fontH*25);
    
    renderText("=== Misc. ==", 10, 10+m_fontH*27);
    auto regIME{registers->getIme()};
    renderText("IME: "+std::to_string(regIME)+" | "+(regIME ? "on" : "off"), 10, 10+m_fontH*28);
    renderText("============", 10, 10+m_fontH*29);
}

void DebugWindow::updateOpcodeValue(const CPU *cpu)
{
    renderText("===== Opcode ====", 10, 10+m_fontH*31);
    renderText("Value: "+toHexStr(cpu->getCurrentOpcode()), 10, 10+m_fontH*32);
    renderText("Name:  "+OpcodeNames::get(cpu->getCurrentOpcode() >> 24), 10, 10+m_fontH*33);
    renderText("Size:  "+std::to_string(cpu->getCurrentOpcodeSize()), 10, 10+m_fontH*34);
    renderText("=================", 10, 10+m_fontH*35);
}

void DebugWindow::updateMemoryValues(const Memory *memory)
{
    // Disable this?

    //for (uint32_t i{}; i <= 0xffff; ++i)
    //    renderText(toHexStr(memory->get(i, false), 2, false), 500+m_fontW*(i%0x100), 10+m_fontH*(i/0x100));

    renderText("= Memory-mapped Registers =", 10, 10+m_fontH*37);

    auto regIF{memory->get(REGISTER_ADDR_IF, false)};
    renderText("IF: "+toHexStr(regIF, 2)+" | "+alignRight(std::to_string(regIF), ' ', 3)+" | "+toBinStr(regIF, 8), 10, 10+m_fontH*38);
    auto regIE{memory->get(REGISTER_ADDR_IE, false)};
    renderText("IE: "+toHexStr(regIE, 2)+" | "+alignRight(std::to_string(regIE), ' ', 3)+" | "+toBinStr(regIE, 8), 10, 10+m_fontH*39);
    renderText("===========================", 10, 10+m_fontH*40);
}

DebugWindow::~DebugWindow()
{
    delete m_fontManager;

    SDL_DestroyWindow(m_window);
    SDL_DestroyRenderer(m_renderer);

    Logger::info("Debug window destroyed");
}

