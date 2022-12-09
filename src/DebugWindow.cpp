#include "DebugWindow.h"
#include "Logger.h"
#include "string_formatting.h"
#include "opcode_names.h"

#include <bitset>
#include <stdint.h>

//#define DEBUG_TEXT_USE_COLORS

DebugWindow::DebugWindow(FontLoader* fontLdr, int x, int y)
{
    m_window = SDL_CreateWindow("Debugger", x, y, 0, 0, 0);

    if (!m_window)
        Logger::fatal("Failed to create debugger window");

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);

    if (!m_renderer)
        Logger::fatal("Failed to create renderer for debugger");

    m_textRend = std::unique_ptr<TextRenderer>(new TextRenderer{m_renderer, fontLdr});

    SDL_SetWindowSize(m_window, TEXT_PADDING_PX*2+m_textRend->getCharW()*40, TEXT_PADDING_PX*2+m_textRend->getCharH()*49);
    SDL_HideWindow(m_window);

    Logger::info("Debug window created");
}

void DebugWindow::updateRegisterValues(const Registers *registers)
{
    m_content+= "===== 8-bit registers ====\n";
    auto regA{registers->getA()};
    m_content+= "A: "+toHexStr(regA, 2)+" | "+alignRight(std::to_string(regA), ' ', 3)+" | "+toBinStr(regA, 8)+'\n';
    auto regB{registers->getB()};
    m_content+= "B: "+toHexStr(regB, 2)+" | "+alignRight(std::to_string(regB), ' ', 3)+" | "+toBinStr(regB, 8)+'\n';
    auto regC{registers->getC()};
    m_content+= "C: "+toHexStr(regC, 2)+" | "+alignRight(std::to_string(regC), ' ', 3)+" | "+toBinStr(regC, 8)+'\n';
    auto regD{registers->getD()};
    m_content+= "D: "+toHexStr(regD, 2)+" | "+alignRight(std::to_string(regD), ' ', 3)+" | "+toBinStr(regD, 8)+'\n';
    auto regE{registers->getE()};
    m_content+= "E: "+toHexStr(regE, 2)+" | "+alignRight(std::to_string(regE), ' ', 3)+" | "+toBinStr(regE, 8)+'\n';
    auto regF{registers->getF()};
    m_content+= "F: "+toHexStr(regF, 2)+" | "+alignRight(std::to_string(regF), ' ', 3)+" | "+toBinStr(regF, 8)+'\n';
    auto regH{registers->getH()};
    m_content+= "H: "+toHexStr(regH, 2)+" | "+alignRight(std::to_string(regH), ' ', 3)+" | "+toBinStr(regH, 8)+'\n';
    auto regL{registers->getL()};
    m_content+= "L: "+toHexStr(regL, 2)+" | "+alignRight(std::to_string(regL), ' ', 3)+" | "+toBinStr(regL, 8)+'\n';
    m_content+= "==========================\n";
    m_content+= '\n';

    m_content+= "=========== 16-bit registers ==========\n";
    auto regAF{registers->getAF()};
    m_content+= "AF: "+toHexStr(regAF, 4)+" | "+alignRight(std::to_string(regAF), ' ', 5)+" | "+toBinStr(regAF, 16)+'\n';
    auto regBC{registers->getBC()};
    m_content+= "BC: "+toHexStr(regBC, 4)+" | "+alignRight(std::to_string(regBC), ' ', 5)+" | "+toBinStr(regBC, 16)+'\n';
    auto regDE{registers->getDE()};
    m_content+= "DE: "+toHexStr(regDE, 4)+" | "+alignRight(std::to_string(regDE), ' ', 5)+" | "+toBinStr(regDE, 16)+'\n';
    auto regHL{registers->getHL()};
    m_content+= "HL: "+toHexStr(regHL, 4)+" | "+alignRight(std::to_string(regHL), ' ', 5)+" | "+toBinStr(regHL, 16)+'\n';
    auto regSP{registers->getSP()};
    m_content+= "SP: "+toHexStr(regSP, 4)+" | "+alignRight(std::to_string(regSP), ' ', 5)+" | "+toBinStr(regSP, 16)+'\n';
    auto regPC{registers->getPC()};
    m_content+= "PC: "+toHexStr(regPC, 4)+" | "+alignRight(std::to_string(regPC), ' ', 5)+" | "+toBinStr(regPC, 16)+'\n';
    m_content+= "=======================================\n";
    m_content+= '\n';

    m_content+= "====== Flags ======\n";
    auto flagZ{registers->getZeroFlag()};
    m_content+= "Zero:       "+std::to_string(flagZ)+" | "+(flagZ ? "on" : "off")+'\n';
    auto flagN{registers->getNegativeFlag()};
    m_content+= "Negative:   "+std::to_string(flagN)+" | "+(flagN ? "on" : "off")+'\n';
    auto flagH{registers->getHalfCarryFlag()};
    m_content+= "Half Carry: "+std::to_string(flagH)+" | "+(flagH ? "on" : "off")+'\n';
    auto flagC{registers->getCarryFlag()};
    m_content+= "Carry:      "+std::to_string(flagC)+" | "+(flagC ? "on" : "off")+'\n';
    m_content+= "===================\n";
    m_content+= '\n';
    
    m_content+= "=== Misc. ==\n";
    auto regIME{registers->getIme()};
    m_content+= "IME: "+std::to_string(regIME)+" | "+(regIME ? "on" : "off")+'\n';
    m_content+= "============\n";
    m_content+= '\n';
}

void DebugWindow::updateOpcodeValue(const CPU *cpu)
{
    m_content+= "===== Opcode ====\n";
    m_content+= "Value: "+toHexStr(cpu->getCurrentOpcode())+'\n';
    m_content+= "Name:  "+OpcodeNames::get(cpu->getCurrentOpcode() >> 24, cpu->isPrefixedOpcode())+'\n';
    m_content+= "Size:  "+std::to_string(cpu->getCurrentOpcodeSize())+'\n';
    m_content+= "Pref.: "+std::string(cpu->isPrefixedOpcode() ? "yes" : "no")+'\n';
    m_content+= "=================\n";
    m_content+= '\n';
}

void DebugWindow::updateMemoryValues(Memory *memory)
{
    // Disable this?

    //for (uint32_t i{}; i <= 0xffff; ++i)
    //    renderLine(toHexStr(memory->get(i, false), 2, false), 500+m_fontW*(i%0x100), 10+m_fontH*(i/0x100));

    m_content+= "== Memory-mapped Registers ==\n";
    auto regIE{memory->get(REGISTER_ADDR_IE, false)};
    m_content+= "IE:   "+toHexStr(regIE, 2)+" | "+alignRight(std::to_string(regIE), ' ', 3)+" | "+toBinStr(regIE, 8)+'\n';
    auto regIF{memory->get(REGISTER_ADDR_IF, false)};
    m_content+= "IF:   "+toHexStr(regIF, 2)+" | "+alignRight(std::to_string(regIF), ' ', 3)+" | "+toBinStr(regIF, 8)+'\n';
    auto regLCDC{memory->get(REGISTER_ADDR_LCDC)};
    m_content+= "LCDC: "+toHexStr(regLCDC, 2)+" | "+alignRight(std::to_string(regLCDC), ' ', 3)+" | "+toBinStr(regLCDC, 8)+'\n';
    auto regSTAT{memory->get(REGISTER_ADDR_LCDSTAT)};
    m_content+= "STAT: "+toHexStr(regSTAT, 2)+" | "+alignRight(std::to_string(regSTAT), ' ', 3)+" | "+toBinStr(regSTAT, 8)+'\n';
    auto regLY{memory->get(REGISTER_ADDR_LY, false)};
    m_content+= "LY:   "+toHexStr(regLY, 2)+" | "+alignRight(std::to_string(regLY), ' ', 3)+" | "+toBinStr(regLY, 8)+'\n';
    auto regDIV{memory->get(REGISTER_ADDR_DIV, false)};
    m_content+= "DIV:  "+toHexStr(regDIV, 2)+" | "+alignRight(std::to_string(regDIV), ' ', 3)+" | "+toBinStr(regDIV, 8)+'\n';
    auto regTIMA{memory->get(REGISTER_ADDR_TIMA, false)};
    m_content+= "TIMA: "+toHexStr(regTIMA, 2)+" | "+alignRight(std::to_string(regTIMA), ' ', 3)+" | "+toBinStr(regTIMA, 8)+'\n';
    auto regTMA{memory->get(REGISTER_ADDR_TMA, false)};
    m_content+= "TMA:  "+toHexStr(regTMA, 2)+" | "+alignRight(std::to_string(regTMA), ' ', 3)+" | "+toBinStr(regTMA, 8)+'\n';
    auto regTAC{memory->get(REGISTER_ADDR_TAC, false)};
    m_content+= "TAC:  "+toHexStr(regTAC, 2)+" | "+alignRight(std::to_string(regTAC), ' ', 3)+" | "+toBinStr(regTAC, 8)+'\n';
    m_content+= "=============================";
}

DebugWindow::~DebugWindow()
{
    SDL_DestroyWindow(m_window);
    SDL_DestroyRenderer(m_renderer);

    Logger::info("Debug window destroyed");
}

