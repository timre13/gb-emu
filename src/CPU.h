#ifndef CPU_H_
#define CPU_H_

#include "config.h"

#include "common.h"
#include "bit_magic.h"

#include "Registers.h"
#include "Memory.h"
#include "Logger.h"
#include "string_formatting.h"

using opcode_t = uint32_t;

class CPU final
{
private:
    Registers       *m_registers{nullptr};
    Memory          *m_memoryPtr{nullptr};

    int             m_opcodeSize{};

    opcode_t        m_currentOpcode{};

    // After executing an instruction the PC is incremented.
    // After a JMP-like opcode we should not increment it,
    // because we need to be at the address we jumped to.
    bool            m_wasJump{};
    // The IMA has to be set after the instruction following EI
    bool            m_wasEiInstruction{};

    static constexpr uint16_t m_interruptHandlers[]{
        0x40, // V-Blank
        0x48, // LCDC Status
        0x50, // Timer
        0x58, // Serial
        0x60  // Joypad
    };

public:
    CPU(Memory *memory);
    ~CPU();

    inline Registers* getRegisters()             { return m_registers; }
    inline const Registers* getRegisters() const { return m_registers; }

    void fetchOpcode();
    inline opcode_t getCurrentOpcode() const     { return m_currentOpcode; }
    inline void stepPC()                         { if (m_wasJump) return; m_registers->setPC(m_registers->getPC()+m_opcodeSize); }
    inline int getCurrentOpcodeSize() const      { return m_opcodeSize; }
    void emulateCurrentOpcode();
    void enableImaIfNeeded()
    {
        // If there was an EI instruction and it is not the current one,
        // so this is the instruction after the EI
        if (m_wasEiInstruction && (((m_currentOpcode & 0xff000000) >> 24) != 0xfb))
        {
            enableIterrupts();
            m_wasEiInstruction = false;
        }
    }

    void handleInterrupts();

private:
    //--------- instructions --------------
    // r8   - 8-bit register
    // r16  - general purpose 16-bit registers
    // n8   - 8-bit integer
    // n16  - 16-bit integer
    // e8   - 8-bit offset (signed) (-128 to 127)
    // u3   - 3-bit unsigned integer (0 to 7)
    // cc   - conditional codes (Z, NZ, C or NC)
    // vec  - RST vector (0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30 and 0x38)

    using r8    =  Registers::r8; // 8-bit register (enum class)
    using r16   = Registers::r16; // 16-bit register (enum class)
    using n8    =        uint8_t; // constant
    using n16   =       uint16_t; // constant
    using e8    =         int8_t; // offset
    using u3    =        uint8_t; // constant
    using cc    = Registers::flag;// condition (enum class)
    using vec   =        uint8_t; // address

    //=========================================================================
    /*
     * Functions to help implement the instructions.
     * Suffixes of 8 or 16 denotes the bitness of the registers used in that function.
     * The F suffix means that the function sets the appropriate flags to the needed value.
     * The affected flags are shown before the functions.
     *
     * Z | N | H | C
     * e | e | a | a
     * r | g | l | r
     * o | a | f | r
     *   | t |   | y
     *   | i | c |
     *   | v | a |
     *   | e | r |
     *   |   | r |
     *   |   | y |
     */

    inline void f()
    {
    }

    // Z0H-
    inline void incrementRegister8F(r8 reg)
    {
        m_registers->setHalfCarryFlag(wouldAddHalfCarry8(m_registers->get8(reg), 1));
        m_registers->set8(reg, m_registers->get8(reg)+1);
        m_registers->setZeroFlag(m_registers->get8(reg) == 0);
        m_registers->unsetNegativeFlag();
    }

    // Z1H-
    inline void decrementRegister8F(r8 reg)
    {
        m_registers->setHalfCarryFlag(wouldSubHalfCarry8(m_registers->get8(reg), 1));
        m_registers->set8(reg, m_registers->get8(reg)-1);
        m_registers->setZeroFlag(m_registers->get8(reg) == 0);
        m_registers->setNegativeFlag();
    }

    // ----
    inline void incrementRegister16(r16 reg)
    {
        m_registers->set16(reg, m_registers->get16(reg)+1);
    }

    // ----
    inline void decrementRegister16(r16 reg)
    {
        m_registers->set16(reg, m_registers->get16(reg)-1);
    }

    // Z0HC
    inline void addToRegister8F(r8 reg, uint8_t value)
    {
        m_registers->setHalfCarryFlag(wouldAddHalfCarry8(m_registers->get8(reg), value));
        m_registers->setCarryFlag(wouldAddCarry8(m_registers->get8(reg), value));
        m_registers->set8(reg, m_registers->get8(reg)+value);
        m_registers->setZeroFlag(m_registers->get8(reg) == 0);
        m_registers->unsetNegativeFlag();
    }

    // ----
    inline void setRegister8(r8 reg, uint8_t value)
    {
        m_registers->set8(reg, value);
    }

    // ----
    inline void setRegister16(r16 reg, uint16_t value)
    {
        m_registers->set16(reg, value);
    }

    // Z1HC
    inline void subFromRegister8F(r8 reg, uint8_t value)
    {
        m_registers->setHalfCarryFlag(wouldSubHalfCarry8(m_registers->get8(reg), value));
        m_registers->setCarryFlag(wouldSubCarry8(m_registers->get8(reg), value));
        m_registers->set8(reg, m_registers->get8(reg)-value);
        m_registers->setZeroFlag(m_registers->get8(reg) == 0);
        m_registers->unsetNegativeFlag();
    }

    // -0HC
    inline void addRegister16ToRegister16F(r16 dest, r16 src)
    {
        m_registers->setHalfCarryFlag(wouldAddHalfCarry16(m_registers->get16(dest), m_registers->get16(src)));
        m_registers->setCarryFlag(wouldAddCarry16(m_registers->get16(dest), m_registers->get16(src)));
        m_registers->set16(dest, m_registers->get16(dest)+m_registers->get16(src));
        m_registers->unsetNegativeFlag();
    }

    // ----
    inline void setValueAtAddressInRegister16(r16 reg, uint8_t value)
    {
        m_memoryPtr->set(m_registers->get16(reg), value);
    }

    // ----
    inline void setValueAtAddressInRegister16ToRegister8(r16 addr, r8 val)
    {
        m_memoryPtr->set(m_registers->get16(addr), m_registers->get8(val));
    }

    // ----
    inline void setValueAtAddress(uint16_t address, uint8_t value)
    {
        m_memoryPtr->set(address, value);
    }

    // ----
    inline void setValueAtAddressToRegister8(uint16_t addr, r8 val)
    {
        m_memoryPtr->set(addr, m_registers->get8(val));
    }

    // ----
    inline void setRegister8ToValueAtAddressInRegister16(r8 destination, r16 sourceAddressRegister)
    {
        m_registers->set8(destination, m_memoryPtr->get(m_registers->get16(sourceAddressRegister)));
    }

    // ----
    inline void setRegister8ToRegister8(r8 destination, r8 source)
    {
        m_registers->set8(destination, m_registers->get8(source));
    }

    // Z0H-
    inline void incrementValueAtAddressInRegister16F(r16 addr)
    {
        m_registers->setHalfCarryFlag(wouldAddHalfCarry8(m_memoryPtr->get(m_registers->get16(addr)), 1));
        m_memoryPtr->set(m_registers->get16(addr), m_memoryPtr->get(m_registers->get16(addr))+1);
        m_registers->setZeroFlag(m_memoryPtr->get(m_registers->get16(addr)) == 0);
        m_registers->unsetNegativeFlag();
    }

    // Z1H-
    inline void decrementValueAtAddressInRegister16F(r16 addr)
    {
        m_registers->setHalfCarryFlag(wouldSubHalfCarry8(m_memoryPtr->get(m_registers->get16(addr)), 1));
        m_memoryPtr->set(m_registers->get16(addr), m_memoryPtr->get(m_registers->get16(addr))-1);
        m_registers->setZeroFlag(m_memoryPtr->get(m_registers->get16(addr)) == 0);
        m_registers->setNegativeFlag();
    }

    // Z0HC
    inline void addRegister8ToRegister8F(r8 dest, r8 src)
    {
        m_registers->setHalfCarryFlag(wouldAddHalfCarry8(m_registers->get8(dest), m_registers->get8(src)));
        m_registers->setCarryFlag(wouldAddCarry8(m_registers->get8(dest), m_registers->get8(src)));
        m_registers->set8(dest, m_registers->get8(dest)+m_registers->get8(src));
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->unsetNegativeFlag();
    }

    // Z0HC
    inline void addValueAtAddressInRegister16ToRegister8F(r8 dest, r16 src)
    {
        m_registers->setHalfCarryFlag(wouldAddHalfCarry8(m_registers->get8(dest), m_memoryPtr->get(m_registers->get16(src))));
        m_registers->setCarryFlag(wouldAddCarry8(m_registers->get8(dest), m_memoryPtr->get(m_registers->get16(src))));
        m_registers->set8(dest, m_registers->get8(dest)+m_memoryPtr->get(m_registers->get16(src)));
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->unsetNegativeFlag();
    }

    // Z0HC
    inline void addRegister8AndCarryFlagToRegister8F(r8 dest, r8 src)
    {
        m_registers->setHalfCarryFlag(wouldAddHalfCarry8(m_registers->get8(dest), m_registers->get8(src)+m_registers->getCarryFlag()));
        m_registers->setCarryFlag(wouldAddCarry8(m_registers->get8(dest), m_registers->get8(src)+m_registers->getCarryFlag()));
        m_registers->set8(dest, m_registers->get8(dest)+m_registers->get8(src)+m_registers->getCarryFlag());
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->unsetNegativeFlag();
    }

    // Z1HC
    inline void subRegister8FromRegister8F(r8 dest, r8 src)
    {
        m_registers->setHalfCarryFlag(wouldSubHalfCarry8(m_registers->get8(dest), m_registers->get8(src)));
        m_registers->setCarryFlag(wouldSubCarry8(m_registers->get8(dest), m_registers->get8(src)));
        m_registers->set8(dest, m_registers->get8(dest)-m_registers->get8(src));
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->setNegativeFlag();
    }

    // Z1HC
    inline void subRegister8AndCarryFlagFromRegister8F(r8 dest, r8 src)
    {
        m_registers->setHalfCarryFlag(wouldSubHalfCarry8(m_registers->get8(dest), m_registers->get8(src)+m_registers->getCarryFlag()));
        m_registers->setCarryFlag(wouldSubCarry8(m_registers->get8(dest), m_registers->get8(src)+m_registers->getCarryFlag()));
        m_registers->set8(dest, m_registers->get8(dest)-m_registers->get8(src)-m_registers->getCarryFlag());
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->setNegativeFlag();
    }

    // Z010
    inline void andRegister8AndRegister8F(r8 dest, r8 src)
    {
        m_registers->set8(dest, m_registers->get8(dest) & m_registers->get8(src));
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->unsetNegativeFlag();
        m_registers->setHalfCarryFlag();
        m_registers->unsetCarryFlag();
    }

    // Z000
    inline void xorRegister8AndRegister8F(r8 dest, r8 src)
    {
        m_registers->set8(dest, m_registers->get8(dest) ^ m_registers->get8(src));
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();
        m_registers->unsetCarryFlag();
    }

    // Z000
    inline void orRegister8AndRegister8F(r8 dest, r8 src)
    {
        m_registers->set8(dest, m_registers->get8(dest) | m_registers->get8(src));
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();
        m_registers->unsetCarryFlag();
    }

    // Z1HC
    inline void cpRegister8AndRegister8F(r8 reg1, r8 reg2)
    {
        m_registers->setZeroFlag(m_registers->get8(reg1) == m_registers->get8(reg2));
        m_registers->setNegativeFlag();
        m_registers->setHalfCarryFlag(wouldSubHalfCarry8(m_registers->get8(reg1), m_registers->get8(reg2)));
        m_registers->setCarryFlag(wouldSubCarry8(m_registers->get8(reg1), m_registers->get8(reg2)));
    }

    // Z0HC
    inline void addValueAndCarryFlagToRegister8F(r8 dest, uint8_t val)
    {
        m_registers->setHalfCarryFlag(wouldAddHalfCarry8(m_registers->get8(dest), val+m_registers->getCarryFlag()));
        m_registers->setCarryFlag(wouldAddCarry8(m_registers->get8(dest), val+m_registers->getCarryFlag()));
        m_registers->set8(dest, m_registers->get8(dest)+val+m_registers->getCarryFlag());
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->unsetNegativeFlag();
    }

    // Z1HC
    inline void subValueAndCarryFlagFromRegister8F(r8 dest, uint8_t val)
    {
        m_registers->setHalfCarryFlag(wouldSubHalfCarry8(m_registers->get8(dest), val+m_registers->getCarryFlag()));
        m_registers->setCarryFlag(wouldSubCarry8(m_registers->get8(dest), val+m_registers->getCarryFlag()));
        m_registers->set8(dest, m_registers->get8(dest)-val-m_registers->getCarryFlag());
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->setNegativeFlag();
    }

    // Z010
    inline void andValueAndRegister8F(r8 dest, uint8_t val)
    {
        m_registers->set8(dest, m_registers->get8(dest) & val);
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->unsetNegativeFlag();
        m_registers->setHalfCarryFlag();
        m_registers->unsetCarryFlag();
    }

    // Z000
    inline void xorValueAndRegister8F(r8 dest, uint8_t val)
    {
        m_registers->set8(dest, m_registers->get8(dest) ^ val);
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();
        m_registers->unsetCarryFlag();
    }

    // Z000
    inline void orValueAndRegister8F(r8 dest, uint8_t val)
    {
        m_registers->set8(dest, m_registers->get8(dest) | val);
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();
        m_registers->unsetCarryFlag();
    }

    // ----
    inline void setRegister16ToRegister16(r16 dest, r16 src)
    {
        m_registers->set16(dest, m_registers->get16(src));
    }

    // Z1HC
    inline void cpRegister8AndValue(r8 reg, uint8_t val)
    {
        m_registers->setZeroFlag(m_registers->get8(reg) == val);
        m_registers->setNegativeFlag();
        m_registers->setHalfCarryFlag(wouldSubHalfCarry8(m_registers->get8(reg), val));
        m_registers->setCarryFlag(wouldSubCarry8(m_registers->get8(reg), val));
    }

    // 000C
    inline void rotateRegister8BitsLeftF(r8 reg)
    {
        m_registers->set8(reg,  (m_registers->get8(reg) << 1) | (m_registers->get8(reg) >> 7));

        m_registers->unsetZeroFlag();
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();
        // FIXME: Set the carry flag according to the result
    }

    // 000C
    inline void rotateRegister8BitsRightF(r8 reg)
    {
        m_registers->set8(reg,  (m_registers->get8(reg) >> 1) | (m_registers->get8(reg) << 7));

        m_registers->unsetZeroFlag();
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();
        // FIXME: Set the carry flag according to the result
    }

    // 000C
    inline void rotateRegister8BitsLeftThroughCarryFlagF(r8 reg)
    {
        auto regVal{m_registers->get8(reg)};

        m_registers->set8(reg,  (regVal << 1) | m_registers->getCarryFlag());
        m_registers->setCarryFlag(regVal >> 7);

        m_registers->unsetZeroFlag();
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();
    }

    // 000C
    inline void rotateRegister8BitsRightThroughCarryFlagF(r8 reg)
    {
        auto regVal{m_registers->get8(reg)};

        m_registers->set8(reg,  (regVal >> 1) | (m_registers->getCarryFlag() << 7));
        m_registers->setCarryFlag(regVal & 1);

        m_registers->unsetZeroFlag();
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();
    }

    // ----
    inline void relativeJump(e8 offset)
    {
        jpToAddress(m_registers->getPC()+offset+2);
    }

    // -11-
    inline void complementRegister8F(r8 reg)
    {
        //                    VVV
        m_registers->set8(reg, ~m_registers->get8(reg));

        m_registers->setNegativeFlag();
        m_registers->setHalfCarryFlag();
    }

    // ----
    inline void jpToAddress(n16 addr)
    {
        m_registers->setPC(addr);
        m_wasJump = true;
    }

    // ----
    inline void jpToAddressInRegister16(r16 reg)
    {
        jpToAddress(m_registers->get16(reg));
    }

    // ----
    inline void jpIf(cc cond, n16 addr)
    {
        if (m_registers->getFlag(cond))
            jpToAddress(addr);
    }

    // ----
    inline void relativeJumpIf(cc cond, e8 offset)
    {
        if (m_registers->getFlag(cond))
            relativeJump(offset);
    }

    // ----
    inline void ret()
    {
        jpToAddress(pop16());
    }

    // ----
    inline void retIf(cc cond)
    {
        if (m_registers->getFlag(cond))
            ret();
    }

    // ----
    inline n8 pop()
    {
        auto ret{m_memoryPtr->get(m_registers->getSP())};
        m_registers->incrementSP();
        return ret;
    }

    // ----
    inline n16 pop16()
    {
        auto ret{m_memoryPtr->get16(m_registers->getSP())};
        m_registers->incrementSP(2);
        return ret;
    }

    // ----
    inline void push(n16 val)
    {
        m_registers->decrementSP();
        m_memoryPtr->set(m_registers->getSP(), val);
    }

    // ----
    inline void push16(n8 val)
    {
        m_registers->decrementSP(2);
        m_memoryPtr->set16(m_registers->getSP(), val);
    }

    // ----
    inline void pushRegister16(r16 reg)
    {
        push16(m_registers->get16(reg));
    }

    // ----
    inline void call(n16 addr)
    {
        push16(m_registers->getPC());
        jpToAddress(addr);
    }

    // ----
    inline void callIf(cc cond, n16 addr)
    {
        if (m_registers->getFlag(cond))
            call(addr);
    }

    // ----
    inline void callVector(vec addr)
    {
        // Call the address stored in a jump vector
        call(m_memoryPtr->get16(addr));
    }

    // ----
    inline void disableInterrupts()
    {
        m_registers->unsetIme();
    }

    // ----
    inline void enableIterrupts()
    {
        m_registers->setIme();
    }

    // Z-0C
    inline void decimalAdjustAccumulator()
    {
        if (m_registers->getNegativeFlag()) // After a substraction
        {
            if (m_registers->getCarryFlag())
                m_registers->setA(m_registers->getA() - 0x60);
            if (m_registers->getHalfCarryFlag())
                m_registers->setA(m_registers->getA() - 0x06);
        }
        else // After an addition
        {
            if (m_registers->getCarryFlag() || m_registers->getA() > 0x99)
            {
                m_registers->setA(m_registers->getA() + 0x60);
                m_registers->setCarryFlag();
            }
            if (m_registers->getHalfCarryFlag() || (m_registers->getA() & 0x0f) > 0x09)
                m_registers->setA(m_registers->getA() + 0x06);
        }

        m_registers->setZeroFlag(m_registers->getA() == 0);
        m_registers->unsetHalfCarryFlag();
    }
    
    inline void ILLEGAL_INSTRUCTION(opcode_t opcode)
    {
        // The Z80-like processors do not crash the system when
        // encountering an illegal instruction, so just report it.
        Logger::warning("Illegal instruction: "+toHexStr(opcode));
    }

    //=========================================================================

    inline void i_0x00()        { /* NOP */ }
    inline void i_0x01(n16 x)   { setRegister16(r16::BC, x); }
    inline void i_0x02()        { setValueAtAddressInRegister16ToRegister8(r16::BC, r8::A); }
    inline void i_0x03()        { incrementRegister16(r16::BC); }
    inline void i_0x04()        { incrementRegister8F(r8::B); }
    inline void i_0x05()        { decrementRegister8F(r8::B); }
    inline void i_0x06(n8 x)    { setRegister8(r8::B, x); }
    inline void i_0x07()        { rotateRegister8BitsLeftF(r8::A); }
    inline void i_0x08(n16 x)   { m_memoryPtr->set16(x, m_registers->getSP()); }
    inline void i_0x09()        { addRegister16ToRegister16F(r16::HL, r16::BC); }
    inline void i_0x0a()        { setRegister8ToValueAtAddressInRegister16(r8::A, r16::BC); }
    inline void i_0x0b()        { decrementRegister16(r16::BC); }
    inline void i_0x0c()        { incrementRegister8F(r8::C); }
    inline void i_0x0d()        { decrementRegister8F(r8::C); }
    inline void i_0x0e(n8 x)    { setRegister8(r8::C, x); }
    inline void i_0x0f()        { rotateRegister8BitsRightF(r8::A); }
    inline void i_0x10()        { UNIMPLEMENTED(); }
    inline void i_0x11(n16 x)   { setRegister16(r16::DE, x); }
    inline void i_0x12()        { setValueAtAddressInRegister16ToRegister8(r16::DE, r8::A); }
    inline void i_0x13()        { incrementRegister16(r16::DE); }
    inline void i_0x14()        { incrementRegister8F(r8::D); }
    inline void i_0x15()        { decrementRegister8F(r8::D); }
    inline void i_0x16(n8 x)    { setRegister8(r8::D, x); }
    inline void i_0x17()        { rotateRegister8BitsLeftThroughCarryFlagF(r8::A); }
    inline void i_0x18(e8 x)    { relativeJump(x); }
    inline void i_0x19()        { addRegister16ToRegister16F(r16::HL, r16::DE); }
    inline void i_0x1a()        { setRegister8ToValueAtAddressInRegister16(r8::A, r16::DE); }
    inline void i_0x1b()        { decrementRegister16(r16::DE); }
    inline void i_0x1c()        { incrementRegister8F(r8::E); }
    inline void i_0x1d()        { decrementRegister8F(r8::E); }
    inline void i_0x1e(n8 x)    { setRegister8(r8::E, x); }
    inline void i_0x1f()        { rotateRegister8BitsRightThroughCarryFlagF(r8::A); }
    inline void i_0x20(e8 x)    { relativeJumpIf(cc::NZ, x); }
    inline void i_0x21(n16 x)   { setRegister16(r16::HL, x); }
    inline void i_0x22()        { setValueAtAddressInRegister16(r16::HL, m_registers->getA()); incrementRegister16(r16::HL); }
    inline void i_0x23()        { incrementRegister16(r16::HL); }
    inline void i_0x24()        { incrementRegister8F(r8::H); }
    inline void i_0x25()        { decrementRegister8F(r8::H); }
    inline void i_0x26(n8 x)    { setRegister8(r8::H, x); }
    inline void i_0x27()        { decimalAdjustAccumulator(); }
    inline void i_0x28(e8 x)    { relativeJumpIf(cc::Z, x); }
    inline void i_0x29()        { addRegister16ToRegister16F(r16::HL, r16::HL); }
    inline void i_0x2a()        { setRegister8ToValueAtAddressInRegister16(r8::A, r16::HL); incrementRegister16(r16::HL); }
    inline void i_0x2b()        { decrementRegister16(r16::HL); }
    inline void i_0x2c()        { incrementRegister8F(r8::L); }
    inline void i_0x2d()        { decrementRegister8F(r8::L); }
    inline void i_0x2e(n8 x)    { setRegister8(r8::L, x); }
    inline void i_0x2f()        { complementRegister8F(r8::A); }
    inline void i_0x30(e8 x)    { relativeJumpIf(cc::NC, x); }
    inline void i_0x31(n16 x)   { m_registers->setSP(x); }
    inline void i_0x32()        { setValueAtAddressInRegister16ToRegister8(r16::HL, r8::A); decrementRegister16(r16::HL); }
    inline void i_0x33()        { incrementRegister16(r16::SP); }
    inline void i_0x34()        { incrementValueAtAddressInRegister16F(r16::HL); }
    inline void i_0x35()        { decrementValueAtAddressInRegister16F(r16::HL); }
    inline void i_0x36(n8 x)    { setValueAtAddressInRegister16(r16::HL, x); }
    inline void i_0x37()        { m_registers->unsetNegativeFlag(); m_registers->unsetHalfCarryFlag(); m_registers->setHalfCarryFlag(); }
    inline void i_0x38(e8 x)    { relativeJumpIf(cc::C, x); }
    inline void i_0x39()        { addRegister16ToRegister16F(r16::HL, r16::SP); }
    inline void i_0x3a()        { setRegister8ToValueAtAddressInRegister16(r8::A, r16::HL); decrementRegister16(r16::HL); }
    inline void i_0x3b()        { decrementRegister16(r16::SP); }
    inline void i_0x3c()        { incrementRegister8F(r8::A); }
    inline void i_0x3d()        { decrementRegister8F(r8::A); }
    inline void i_0x3e(n8 x)    { setRegister8(r8::A, x); }
    inline void i_0x3f()        { m_registers->unsetNegativeFlag(); m_registers->unsetHalfCarryFlag(); m_registers->setCarryFlag(!m_registers->getHalfCarryFlag()); }
    inline void i_0x40()        { setRegister8ToRegister8(r8::B, r8::B); }
    inline void i_0x41()        { setRegister8ToRegister8(r8::B, r8::C); }
    inline void i_0x42()        { setRegister8ToRegister8(r8::B, r8::D); }
    inline void i_0x43()        { setRegister8ToRegister8(r8::B, r8::E); }
    inline void i_0x44()        { setRegister8ToRegister8(r8::B, r8::H); }
    inline void i_0x45()        { setRegister8ToRegister8(r8::B, r8::L); }
    inline void i_0x46()        { setRegister8ToValueAtAddressInRegister16(r8::B, r16::HL); }
    inline void i_0x47()        { setRegister8ToRegister8(r8::B, r8::A); }
    inline void i_0x48()        { setRegister8ToRegister8(r8::C, r8::B); }
    inline void i_0x49()        { setRegister8ToRegister8(r8::C, r8::C); }
    inline void i_0x4a()        { setRegister8ToRegister8(r8::C, r8::D); }
    inline void i_0x4b()        { setRegister8ToRegister8(r8::C, r8::E); }
    inline void i_0x4c()        { setRegister8ToRegister8(r8::C, r8::H); }
    inline void i_0x4d()        { setRegister8ToRegister8(r8::C, r8::L); }
    inline void i_0x4e()        { setRegister8ToValueAtAddressInRegister16(r8::C, r16::HL); }
    inline void i_0x4f()        { setRegister8ToRegister8(r8::C, r8::A); }
    inline void i_0x50()        { setRegister8ToRegister8(r8::D, r8::B); }
    inline void i_0x51()        { setRegister8ToRegister8(r8::D, r8::C); }
    inline void i_0x52()        { setRegister8ToRegister8(r8::D, r8::D); }
    inline void i_0x53()        { setRegister8ToRegister8(r8::D, r8::E); }
    inline void i_0x54()        { setRegister8ToRegister8(r8::D, r8::H); }
    inline void i_0x55()        { setRegister8ToRegister8(r8::D, r8::L); }
    inline void i_0x56()        { setRegister8ToValueAtAddressInRegister16(r8::D, r16::HL); }
    inline void i_0x57()        { setRegister8ToRegister8(r8::D, r8::A); }
    inline void i_0x58()        { setRegister8ToRegister8(r8::E, r8::B); }
    inline void i_0x59()        { setRegister8ToRegister8(r8::E, r8::C); }
    inline void i_0x5a()        { setRegister8ToRegister8(r8::E, r8::D); }
    inline void i_0x5b()        { setRegister8ToRegister8(r8::E, r8::E); }
    inline void i_0x5c()        { setRegister8ToRegister8(r8::E, r8::H); }
    inline void i_0x5d()        { setRegister8ToRegister8(r8::E, r8::L); }
    inline void i_0x5e()        { setRegister8ToValueAtAddressInRegister16(r8::E, r16::HL); }
    inline void i_0x5f()        { setRegister8ToRegister8(r8::E, r8::A); }
    inline void i_0x60()        { setRegister8ToRegister8(r8::H, r8::B); }
    inline void i_0x61()        { setRegister8ToRegister8(r8::H, r8::C); }
    inline void i_0x62()        { setRegister8ToRegister8(r8::H, r8::D); }
    inline void i_0x63()        { setRegister8ToRegister8(r8::H, r8::E); }
    inline void i_0x64()        { setRegister8ToRegister8(r8::H, r8::H); }
    inline void i_0x65()        { setRegister8ToRegister8(r8::H, r8::L); }
    inline void i_0x66()        { setRegister8ToValueAtAddressInRegister16(r8::H, r16::HL); }
    inline void i_0x67()        { setRegister8ToRegister8(r8::H, r8::A); }
    inline void i_0x68()        { setRegister8ToRegister8(r8::L, r8::B); }
    inline void i_0x69()        { setRegister8ToRegister8(r8::L, r8::C); }
    inline void i_0x6a()        { setRegister8ToRegister8(r8::L, r8::D); }
    inline void i_0x6b()        { setRegister8ToRegister8(r8::L, r8::E); }
    inline void i_0x6c()        { setRegister8ToRegister8(r8::L, r8::H); }
    inline void i_0x6d()        { setRegister8ToRegister8(r8::L, r8::L); }
    inline void i_0x6e()        { setRegister8ToValueAtAddressInRegister16(r8::L, r16::HL); }
    inline void i_0x6f()        { setRegister8ToRegister8(r8::L, r8::A); }
    inline void i_0x70()        { setValueAtAddressInRegister16ToRegister8(r16::HL, r8::B); }
    inline void i_0x71()        { setValueAtAddressInRegister16ToRegister8(r16::HL, r8::C); }
    inline void i_0x72()        { setValueAtAddressInRegister16ToRegister8(r16::HL, r8::D); }
    inline void i_0x73()        { setValueAtAddressInRegister16ToRegister8(r16::HL, r8::E); }
    inline void i_0x74()        { setValueAtAddressInRegister16ToRegister8(r16::HL, r8::H); }
    inline void i_0x75()        { setValueAtAddressInRegister16ToRegister8(r16::HL, r8::L); }
    inline void i_0x76()        { UNIMPLEMENTED(); }
    inline void i_0x77()        { setValueAtAddressInRegister16ToRegister8(r16::HL, r8::A); }
    inline void i_0x78()        { setRegister8ToRegister8(r8::A, r8::B); }
    inline void i_0x79()        { setRegister8ToRegister8(r8::A, r8::C); }
    inline void i_0x7a()        { setRegister8ToRegister8(r8::A, r8::D); }
    inline void i_0x7b()        { setRegister8ToRegister8(r8::A, r8::E); }
    inline void i_0x7c()        { setRegister8ToRegister8(r8::A, r8::H); }
    inline void i_0x7d()        { setRegister8ToRegister8(r8::A, r8::L); }
    inline void i_0x7e()        { setRegister8ToValueAtAddressInRegister16(r8::A, r16::HL); }
    inline void i_0x7f()        { setRegister8ToRegister8(r8::A, r8::A); }
    inline void i_0x80()        { addRegister8ToRegister8F(r8::A, r8::B); }
    inline void i_0x81()        { addRegister8ToRegister8F(r8::A, r8::C); }
    inline void i_0x82()        { addRegister8ToRegister8F(r8::A, r8::D); }
    inline void i_0x83()        { addRegister8ToRegister8F(r8::A, r8::E); }
    inline void i_0x84()        { addRegister8ToRegister8F(r8::A, r8::H); }
    inline void i_0x85()        { addRegister8ToRegister8F(r8::A, r8::L); }
    inline void i_0x86()        { addValueAtAddressInRegister16ToRegister8F(r8::A, r16::HL); }
    inline void i_0x87()        { addRegister8ToRegister8F(r8::A, r8::A); }
    inline void i_0x88()        { addRegister8AndCarryFlagToRegister8F(r8::A, r8::B); }
    inline void i_0x89()        { addRegister8AndCarryFlagToRegister8F(r8::A, r8::C); }
    inline void i_0x8a()        { addRegister8AndCarryFlagToRegister8F(r8::A, r8::D); }
    inline void i_0x8b()        { addRegister8AndCarryFlagToRegister8F(r8::A, r8::E); }
    inline void i_0x8c()        { addRegister8AndCarryFlagToRegister8F(r8::A, r8::H); }
    inline void i_0x8d()        { addRegister8AndCarryFlagToRegister8F(r8::A, r8::L); }
    inline void i_0x8e()        { addValueAndCarryFlagToRegister8F(r8::A, m_memoryPtr->get(m_registers->getHL())); }
    inline void i_0x8f()        { addRegister8AndCarryFlagToRegister8F(r8::A, r8::A); }
    inline void i_0x90()        { subRegister8FromRegister8F(r8::A, r8::B); }
    inline void i_0x91()        { subRegister8FromRegister8F(r8::A, r8::C); }
    inline void i_0x92()        { subRegister8FromRegister8F(r8::A, r8::D); }
    inline void i_0x93()        { subRegister8FromRegister8F(r8::A, r8::E); }
    inline void i_0x94()        { subRegister8FromRegister8F(r8::A, r8::H); }
    inline void i_0x95()        { subRegister8FromRegister8F(r8::A, r8::L); }
    inline void i_0x96()        { subFromRegister8F(r8::A, m_memoryPtr->get(m_registers->getHL())); }
    inline void i_0x97()        { subRegister8FromRegister8F(r8::A, r8::A); }
    inline void i_0x98()        { subRegister8AndCarryFlagFromRegister8F(r8::A, r8::B); }
    inline void i_0x99()        { subRegister8AndCarryFlagFromRegister8F(r8::A, r8::C); }
    inline void i_0x9a()        { subRegister8AndCarryFlagFromRegister8F(r8::A, r8::D); }
    inline void i_0x9b()        { subRegister8AndCarryFlagFromRegister8F(r8::A, r8::E); }
    inline void i_0x9c()        { subRegister8AndCarryFlagFromRegister8F(r8::A, r8::H); }
    inline void i_0x9d()        { subRegister8AndCarryFlagFromRegister8F(r8::A, r8::L); }
    inline void i_0x9e()        { subValueAndCarryFlagFromRegister8F(r8::A, m_memoryPtr->get(m_registers->getHL())); }
    inline void i_0x9f()        { subRegister8AndCarryFlagFromRegister8F(r8::A, r8::A); }
    inline void i_0xa0()        { andRegister8AndRegister8F(r8::A, r8::B); }
    inline void i_0xa1()        { andRegister8AndRegister8F(r8::A, r8::C); }
    inline void i_0xa2()        { andRegister8AndRegister8F(r8::A, r8::D); }
    inline void i_0xa3()        { andRegister8AndRegister8F(r8::A, r8::E); }
    inline void i_0xa4()        { andRegister8AndRegister8F(r8::A, r8::H); }
    inline void i_0xa5()        { andRegister8AndRegister8F(r8::A, r8::L); }
    inline void i_0xa6()        { andValueAndRegister8F(r8::A, m_memoryPtr->get(m_registers->getHL())); }
    inline void i_0xa7()        { andRegister8AndRegister8F(r8::A, r8::A); }
    inline void i_0xa8()        { xorRegister8AndRegister8F(r8::A, r8::B); }
    inline void i_0xa9()        { xorRegister8AndRegister8F(r8::A, r8::C); }
    inline void i_0xaa()        { xorRegister8AndRegister8F(r8::A, r8::D); }
    inline void i_0xab()        { xorRegister8AndRegister8F(r8::A, r8::E); }
    inline void i_0xac()        { xorRegister8AndRegister8F(r8::A, r8::H); }
    inline void i_0xad()        { xorRegister8AndRegister8F(r8::A, r8::L); }
    inline void i_0xae()        { xorValueAndRegister8F(r8::A, m_memoryPtr->get(m_registers->getHL())); }
    inline void i_0xaf()        { xorRegister8AndRegister8F(r8::A, r8::A); }
    inline void i_0xb0()        { orRegister8AndRegister8F(r8::A, r8::B); }
    inline void i_0xb1()        { orRegister8AndRegister8F(r8::A, r8::C); }
    inline void i_0xb2()        { orRegister8AndRegister8F(r8::A, r8::D); }
    inline void i_0xb3()        { orRegister8AndRegister8F(r8::A, r8::E); }
    inline void i_0xb4()        { orRegister8AndRegister8F(r8::A, r8::H); }
    inline void i_0xb5()        { orRegister8AndRegister8F(r8::A, r8::L); }
    inline void i_0xb6()        { orValueAndRegister8F(r8::A, m_memoryPtr->get(m_registers->getHL()));}
    inline void i_0xb7()        { orRegister8AndRegister8F(r8::A, r8::A); }
    inline void i_0xb8()        { cpRegister8AndRegister8F(r8::A, r8::B); }
    inline void i_0xb9()        { cpRegister8AndRegister8F(r8::A, r8::C); }
    inline void i_0xba()        { cpRegister8AndRegister8F(r8::A, r8::D); }
    inline void i_0xbb()        { cpRegister8AndRegister8F(r8::A, r8::E); }
    inline void i_0xbc()        { cpRegister8AndRegister8F(r8::A, r8::H); }
    inline void i_0xbd()        { cpRegister8AndRegister8F(r8::A, r8::L); }
    inline void i_0xbe()        { cpRegister8AndValue(r8::A, m_memoryPtr->get(m_registers->getHL())); }
    inline void i_0xbf()        { cpRegister8AndRegister8F(r8::A, r8::A); }
    inline void i_0xc0()        { retIf(cc::NZ); }
    inline void i_0xc1()        { pop(); }
    inline void i_0xc2(n16 x)   { jpIf(cc::NZ, x); }
    inline void i_0xc3(n16 x)   { jpToAddress(x); }
    inline void i_0xc4(n16 x)   { callIf(cc::NZ, x); }
    inline void i_0xc5()        { pushRegister16(r16::BC); }
    inline void i_0xc6(n8 x)    { addToRegister8F(r8::A, x); }
    inline void i_0xc7()        { callVector(0x00); }
    inline void i_0xc8()        { retIf(cc::Z); }
    inline void i_0xc9()        { ret(); }
    inline void i_0xca(n16 x)   { jpIf(cc::Z, x); }
    inline void i_0xcb()        { UNIMPLEMENTED(); Logger::error("Found an CB prefix"); }
    inline void i_0xcc(n16 x)   { callIf(cc::Z, x); }
    inline void i_0xcd(n16 x)   { call(x); }
    inline void i_0xce(n8 x)    { addValueAndCarryFlagToRegister8F(r8::A, x); }
    inline void i_0xcf()        { callVector(0x08); }
    inline void i_0xd0()        { retIf(cc::NC); }
    inline void i_0xd1()        { m_registers->set16(r16::DE, pop16()); }
    inline void i_0xd2(n16 x)   { jpIf(cc::NC, x); }
    inline void i_0xd3()        { ILLEGAL_INSTRUCTION(0xd3); }
    inline void i_0xd4(n16 x)   { callIf(cc::NC, x); }
    inline void i_0xd5()        { pushRegister16(r16::DE); }
    inline void i_0xd6(n8 x)    { subFromRegister8F(r8::A, x); }
    inline void i_0xd7()        { callVector(0x10); }
    inline void i_0xd8()        { retIf(cc::C); }
    inline void i_0xd9()        { enableIterrupts(); ret(); }
    inline void i_0xda(n16 x)   { jpIf(cc::C, x); }
    inline void i_0xdb()        { ILLEGAL_INSTRUCTION(0xdb); }
    inline void i_0xdc(n16 x)   { callIf(cc::C, x); }
    inline void i_0xdd()        { ILLEGAL_INSTRUCTION(0xdd); }
    inline void i_0xde(n8 x)    { subValueAndCarryFlagFromRegister8F(r8::A, x); }
    inline void i_0xdf()        { callVector(0x18); }
    inline void i_0xe0(n8 x)    { setValueAtAddressToRegister8(0xff00+x, r8::A); }
    inline void i_0xe1()        { m_registers->setHL(pop16()); }
    inline void i_0xe2()        { setValueAtAddressToRegister8(0xff00+m_registers->getC(), r8::A); }
    inline void i_0xe3()        { ILLEGAL_INSTRUCTION(0xe3); }
    inline void i_0xe4()        { ILLEGAL_INSTRUCTION(0xe4); }
    inline void i_0xe5()        { pushRegister16(r16::HL); }
    inline void i_0xe6(n8 x)    { andValueAndRegister8F(r8::A, x); }
    inline void i_0xe7()        { callVector(0x20); }
    inline void i_0xe8(e8 x)    { m_registers->incrementSP(x); }
    inline void i_0xe9()        { jpToAddressInRegister16(r16::HL); }
    inline void i_0xea(n8 x)    { setValueAtAddressToRegister8(x, r8::A); }
    inline void i_0xeb()        { ILLEGAL_INSTRUCTION(0xeb); }
    inline void i_0xec()        { ILLEGAL_INSTRUCTION(0xec); }
    inline void i_0xed()        { ILLEGAL_INSTRUCTION(0xed); }
    inline void i_0xee(n8 x)    { xorValueAndRegister8F(r8::A, x); }
    inline void i_0xef()        { callVector(0x28); }
    inline void i_0xf0(n8 x)    { setRegister8(r8::A, m_memoryPtr->get(0xff00+x)); }
    inline void i_0xf1()        { m_registers->setAF(pop16()); }
    inline void i_0xf2()        { setRegister8(r8::A, m_memoryPtr->get(0xff00+m_registers->getC())); }
    inline void i_0xf3()        { disableInterrupts(); }
    inline void i_0xf4()        { ILLEGAL_INSTRUCTION(0xf4); }
    inline void i_0xf5()        { pushRegister16(r16::AF); }
    inline void i_0xf6(n8 x)    { orValueAndRegister8F(r8::A, x); }
    inline void i_0xf7()        { callVector(0x30); }
    inline void i_0xf8(e8 x)    { setRegister16(r16::HL, m_registers->getSP()+x); }
    inline void i_0xf9()        { setRegister16ToRegister16(r16::SP, r16::HL); }
    inline void i_0xfa(n8 x)    { setRegister8(r8::A, x); }
    inline void i_0xfb()        { m_wasEiInstruction = true; }
    inline void i_0xfc()        { ILLEGAL_INSTRUCTION(0xfc); }
    inline void i_0xfd()        { ILLEGAL_INSTRUCTION(0xfd); }
    inline void i_0xfe(n8 x)    { cpRegister8AndValue(r8::A, x); }
    inline void i_0xff()        { callVector(0x38); }
};



#endif /* CPU_H_ */
