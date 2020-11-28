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

#define INTERRUPT_HANDLER_VBLANK   0x40
#define INTERRUPT_HANDLER_LCDCSTAT 0x48
#define INTERRUPT_HANDLER_TIMER    0x50
#define INTERRUPT_HANDLER_SERIAL   0x58
#define INTERRUPT_HANDLER_JOYPAD   0x60

#define JUMP_VECTOR_00 0x00
#define JUMP_VECTOR_08 0x08
#define JUMP_VECTOR_10 0x10
#define JUMP_VECTOR_18 0x18
#define JUMP_VECTOR_20 0x20
#define JUMP_VECTOR_28 0x28
#define JUMP_VECTOR_30 0x30
#define JUMP_VECTOR_38 0x38

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
        INTERRUPT_HANDLER_VBLANK,
        INTERRUPT_HANDLER_LCDCSTAT,
        INTERRUPT_HANDLER_TIMER,
        INTERRUPT_HANDLER_SERIAL,
        INTERRUPT_HANDLER_JOYPAD,
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
    /*
     * Emulates the current opcode and returns the number of M-cycles it took.
     *
     * 1 M-cycle is 4 T-cycles!
     */
    int emulateCurrentOpcode();
    void enableImaIfNeeded()
    {
        // If there was an EI instruction and it is not the current one,
        // so this is the instruction after the EI
        if (m_wasEiInstruction && (m_currentOpcode >> 24 != 0xfb))
        {
            enableInterrupts();
            m_wasEiInstruction = false;
        }
    }

    void handleInterrupts();

private:
    //--------- instructions --------------
    // r8   - 8-bit register
    // r16  - general purpose 16-bit registers
    // u8   - 8-bit unsigned integer
    // u16  - 16-bit unsigned integer
    // i8   - 8-bit offset (signed) (-128 to 127)
    // u3   - 3-bit unsigned integer (0 to 7)
    // cc   - conditional codes (Z, NZ, C or NC)
    // vec  - RST vector (address) (0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30 and 0x38)

    using r8    =  Registers::r8; // 8-bit register (enum class)
    using r16   = Registers::r16; // 16-bit register (enum class)
    using u8    =        uint8_t; // constant (old name: n8)
    using u16   =       uint16_t; // constant (old name: n16)
    using i8    =         int8_t; // offset (old name: e8)
    using u3    =        uint8_t; // constant
    using cc    = Registers::cond;// condition (enum class)
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
    inline int incrementRegister8F(r8 reg)
    {
        m_registers->setHalfCarryFlag(wouldAddHalfCarry8(m_registers->get8(reg), 1));
        m_registers->set8(reg, m_registers->get8(reg)+1);
        m_registers->setZeroFlag(m_registers->get8(reg) == 0);
        m_registers->unsetNegativeFlag();

        return 1;
    }

    // Z1H-
    inline int decrementRegister8F(r8 reg)
    {
        m_registers->setHalfCarryFlag(wouldSubHalfCarry8(m_registers->get8(reg), 1));
        m_registers->set8(reg, m_registers->get8(reg)-1);
        m_registers->setZeroFlag(m_registers->get8(reg) == 0);
        m_registers->setNegativeFlag();

        return 1;
    }

    // ----
    inline int incrementRegister16(r16 reg)
    {
        m_registers->set16(reg, m_registers->get16(reg)+1);

        return 2;
    }

    // ----
    inline int decrementRegister16(r16 reg)
    {
        m_registers->set16(reg, m_registers->get16(reg)-1);

        return 2;
    }

    // Z0HC
    inline int addToRegister8F(r8 reg, u8 value)
    {
        m_registers->setHalfCarryFlag(wouldAddHalfCarry8(m_registers->get8(reg), value));
        m_registers->setCarryFlag(wouldAddCarry8(m_registers->get8(reg), value));
        m_registers->set8(reg, m_registers->get8(reg)+value);
        m_registers->setZeroFlag(m_registers->get8(reg) == 0);
        m_registers->unsetNegativeFlag();

        return 2;
    }

    // ----
    inline int setRegister8(r8 reg, u8 value)
    {
        m_registers->set8(reg, value);

        return 2;
    }

    // ----
    inline int setRegister16(r16 reg, u16 value)
    {
        m_registers->set16(reg, value);

        return 3;
    }

    // Z1HC
    inline int subFromRegister8F(r8 reg, u8 value)
    {
        m_registers->setHalfCarryFlag(wouldSubHalfCarry8(m_registers->get8(reg), value));
        m_registers->setCarryFlag(wouldSubCarry8(m_registers->get8(reg), value));
        m_registers->set8(reg, m_registers->get8(reg)-value);
        m_registers->setZeroFlag(m_registers->get8(reg) == 0);
        m_registers->unsetNegativeFlag();

        return 2;
    }

    // -0HC
    inline int addRegister16ToRegister16F(r16 dest, r16 src)
    {
        m_registers->setHalfCarryFlag(wouldAddHalfCarry16(m_registers->get16(dest), m_registers->get16(src)));
        m_registers->setCarryFlag(wouldAddCarry16(m_registers->get16(dest), m_registers->get16(src)));
        m_registers->set16(dest, m_registers->get16(dest)+m_registers->get16(src));
        m_registers->unsetNegativeFlag();

        return 2;
    }

    // ----
    inline int setValueAtAddressInRegister16(r16 reg, u8 value)
    {
        m_memoryPtr->set(m_registers->get16(reg), value);

        return 3;
    }

    // ----
    inline int setValueAtAddressInRegister16ToRegister8(r16 addr, r8 val)
    {
        m_memoryPtr->set(m_registers->get16(addr), m_registers->get8(val));

        return 2;
    }

    // ----
    inline int setValueAtAddressToRegister8(u16 addr, r8 val)
    {
        m_memoryPtr->set(addr, m_registers->get8(val));

        return 4;
    }

    // ----
    inline int setRegister8ToValueAtAddressInRegister16(r8 destination, r16 sourceAddressRegister)
    {
        m_registers->set8(destination, m_memoryPtr->get(m_registers->get16(sourceAddressRegister)));

        return 2;
    }

    // ----
    inline int setRegister8ToRegister8(r8 destination, r8 source)
    {
        m_registers->set8(destination, m_registers->get8(source));

        return 1;
    }

    // Z0H-
    inline int incrementValueAtAddressInRegister16F(r16 addr)
    {
        m_registers->setHalfCarryFlag(wouldAddHalfCarry8(m_memoryPtr->get(m_registers->get16(addr)), 1));
        m_memoryPtr->set(m_registers->get16(addr), m_memoryPtr->get(m_registers->get16(addr))+1);
        m_registers->setZeroFlag(m_memoryPtr->get(m_registers->get16(addr)) == 0);
        m_registers->unsetNegativeFlag();

        return 3;
    }

    // Z1H-
    inline int decrementValueAtAddressInRegister16F(r16 addr)
    {
        m_registers->setHalfCarryFlag(wouldSubHalfCarry8(m_memoryPtr->get(m_registers->get16(addr)), 1));
        m_memoryPtr->set(m_registers->get16(addr), m_memoryPtr->get(m_registers->get16(addr))-1);
        m_registers->setZeroFlag(m_memoryPtr->get(m_registers->get16(addr)) == 0);
        m_registers->setNegativeFlag();

        return 3;
    }

    // Z0HC
    inline int addRegister8ToRegister8F(r8 dest, r8 src)
    {
        m_registers->setHalfCarryFlag(wouldAddHalfCarry8(m_registers->get8(dest), m_registers->get8(src)));
        m_registers->setCarryFlag(wouldAddCarry8(m_registers->get8(dest), m_registers->get8(src)));
        m_registers->set8(dest, m_registers->get8(dest)+m_registers->get8(src));
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->unsetNegativeFlag();

        return 1;
    }

    // Z0HC
    inline int addValueAtAddressInRegister16ToRegister8F(r8 dest, r16 src)
    {
        m_registers->setHalfCarryFlag(wouldAddHalfCarry8(m_registers->get8(dest), m_memoryPtr->get(m_registers->get16(src))));
        m_registers->setCarryFlag(wouldAddCarry8(m_registers->get8(dest), m_memoryPtr->get(m_registers->get16(src))));
        m_registers->set8(dest, m_registers->get8(dest)+m_memoryPtr->get(m_registers->get16(src)));
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->unsetNegativeFlag();

        return 2;
    }

    // Z0HC
    inline int addRegister8AndCarryFlagToRegister8F(r8 dest, r8 src)
    {
        m_registers->setHalfCarryFlag(wouldAddHalfCarry8(m_registers->get8(dest), m_registers->get8(src)+m_registers->getCarryFlag()));
        m_registers->setCarryFlag(wouldAddCarry8(m_registers->get8(dest), m_registers->get8(src)+m_registers->getCarryFlag()));
        m_registers->set8(dest, m_registers->get8(dest)+m_registers->get8(src)+m_registers->getCarryFlag());
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->unsetNegativeFlag();

        return 2;
    }

    // Z1HC
    inline int subRegister8FromRegister8F(r8 dest, r8 src)
    {
        m_registers->setHalfCarryFlag(wouldSubHalfCarry8(m_registers->get8(dest), m_registers->get8(src)));
        m_registers->setCarryFlag(wouldSubCarry8(m_registers->get8(dest), m_registers->get8(src)));
        m_registers->set8(dest, m_registers->get8(dest)-m_registers->get8(src));
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->setNegativeFlag();

        return 1;
    }

    // Z1HC
    inline int subRegister8AndCarryFlagFromRegister8F(r8 dest, r8 src)
    {
        m_registers->setHalfCarryFlag(wouldSubHalfCarry8(m_registers->get8(dest), m_registers->get8(src)+m_registers->getCarryFlag()));
        m_registers->setCarryFlag(wouldSubCarry8(m_registers->get8(dest), m_registers->get8(src)+m_registers->getCarryFlag()));
        m_registers->set8(dest, m_registers->get8(dest)-m_registers->get8(src)-m_registers->getCarryFlag());
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->setNegativeFlag();

        return 1;
    }

    // Z010
    inline int andRegister8AndRegister8F(r8 dest, r8 src)
    {
        m_registers->set8(dest, m_registers->get8(dest) & m_registers->get8(src));
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->unsetNegativeFlag();
        m_registers->setHalfCarryFlag();
        m_registers->unsetCarryFlag();

        return 1;
    }

    // Z000
    inline int xorRegister8AndRegister8F(r8 dest, r8 src)
    {
        m_registers->set8(dest, m_registers->get8(dest) ^ m_registers->get8(src));
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();
        m_registers->unsetCarryFlag();

        return 1;
    }

    // Z000
    inline int orRegister8AndRegister8F(r8 dest, r8 src)
    {
        m_registers->set8(dest, m_registers->get8(dest) | m_registers->get8(src));
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();
        m_registers->unsetCarryFlag();

        return 1;
    }

    // Z1HC
    inline int cpRegister8AndRegister8F(r8 reg1, r8 reg2)
    {
        m_registers->setZeroFlag(m_registers->get8(reg1) == m_registers->get8(reg2));
        m_registers->setNegativeFlag();
        m_registers->setHalfCarryFlag(wouldSubHalfCarry8(m_registers->get8(reg1), m_registers->get8(reg2)));
        m_registers->setCarryFlag(wouldSubCarry8(m_registers->get8(reg1), m_registers->get8(reg2)));

        return 1;
    }

    // Z0HC
    inline int addValueAndCarryFlagToRegister8F(r8 dest, u8 val)
    {
        m_registers->setHalfCarryFlag(wouldAddHalfCarry8(m_registers->get8(dest), val+m_registers->getCarryFlag()));
        m_registers->setCarryFlag(wouldAddCarry8(m_registers->get8(dest), val+m_registers->getCarryFlag()));
        m_registers->set8(dest, m_registers->get8(dest)+val+m_registers->getCarryFlag());
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->unsetNegativeFlag();

        return 2;
    }

    // Z1HC
    inline int subValueAndCarryFlagFromRegister8F(r8 dest, u8 val)
    {
        m_registers->setHalfCarryFlag(wouldSubHalfCarry8(m_registers->get8(dest), val+m_registers->getCarryFlag()));
        m_registers->setCarryFlag(wouldSubCarry8(m_registers->get8(dest), val+m_registers->getCarryFlag()));
        m_registers->set8(dest, m_registers->get8(dest)-val-m_registers->getCarryFlag());
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->setNegativeFlag();

        return 2;
    }

    // Z010
    inline int andValueAndRegister8F(r8 dest, u8 val)
    {
        m_registers->set8(dest, m_registers->get8(dest) & val);
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->unsetNegativeFlag();
        m_registers->setHalfCarryFlag();
        m_registers->unsetCarryFlag();

        return 2;
    }

    // Z000
    inline int xorValueAndRegister8F(r8 dest, u8 val)
    {
        m_registers->set8(dest, m_registers->get8(dest) ^ val);
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();
        m_registers->unsetCarryFlag();

        return 2;
    }

    // Z000
    inline int orValueAndRegister8F(r8 dest, u8 val)
    {
        m_registers->set8(dest, m_registers->get8(dest) | val);
        m_registers->setZeroFlag(m_registers->get8(dest) == 0);
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();
        m_registers->unsetCarryFlag();

        return 2;
    }

    // ----
    inline int setRegister16ToRegister16(r16 dest, r16 src)
    {
        m_registers->set16(dest, m_registers->get16(src));

        return 2;
    }

    // Z1HC
    inline int cpRegister8AndValue(r8 reg, u8 val)
    {
        m_registers->setZeroFlag(m_registers->get8(reg) == val);
        m_registers->setNegativeFlag();
        m_registers->setHalfCarryFlag(wouldSubHalfCarry8(m_registers->get8(reg), val));
        m_registers->setCarryFlag(wouldSubCarry8(m_registers->get8(reg), val));

        return 2;
    }

    // 000C
    inline int rotateRegister8BitsLeftF(r8 reg)
    {
        m_registers->set8(reg,  (m_registers->get8(reg) << 1) | (m_registers->get8(reg) >> 7));

        m_registers->unsetZeroFlag();
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();
        // FIXME: Set the carry flag according to the result

        return 1;
    }

    // 000C
    inline int rotateARegBitsRightF()
    {
        m_registers->setA((m_registers->getA() >> 1) | (m_registers->getA() << 7));

        m_registers->unsetZeroFlag();
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();
        // FIXME: Set the carry flag according to the result

        return 1;
    }

    // 000C
    inline int rotateARegBitsLeftThroughCarryFlagF()
    {
        auto regVal{m_registers->getA()};

        m_registers->setA((regVal << 1) | m_registers->getCarryFlag());
        m_registers->setCarryFlag(regVal >> 7);

        m_registers->unsetZeroFlag();
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();

        return 1;
    }

    // 000C
    inline int rotateARegBitsRightThroughCarryFlagF()
    {
        auto regVal{m_registers->getA()};

        m_registers->setA((regVal >> 1) | (m_registers->getCarryFlag() << 7));
        m_registers->setCarryFlag(regVal & 1);

        m_registers->unsetZeroFlag();
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();

        return 1;
    }

    // ----
    inline int relativeJump(i8 offset)
    {
        jpToAddress(m_registers->getPC()+offset+2);

        return 3;
    }

    // -11-
    inline int complementRegister8F(r8 reg)
    {
        //                    VVV
        m_registers->set8(reg, ~m_registers->get8(reg));

        m_registers->setNegativeFlag();
        m_registers->setHalfCarryFlag();

        return 1;
    }

    // ----
    inline int jpToAddress(u16 addr)
    {
        m_registers->setPC(addr);
        m_wasJump = true;

        return 4;
    }

    // ----
    inline int jpToAddressInRegister16(r16 reg)
    {
        jpToAddress(m_registers->get16(reg));

        return 1;
    }

    // ----
    inline int jpIf(cc cond, u16 addr)
    {
        if (m_registers->getCondition(cond))
        {
            jpToAddress(addr);

            return 4;
        }

        return 3;
    }

    // ----
    inline int relativeJumpIf(cc cond, i8 offset)
    {
        if (m_registers->getCondition(cond))
        {
            relativeJump(offset);

            return 3;
        }

        return 2;
    }

    // ----
    inline int ret()
    {
        jpToAddress(pop16());

        return 4;
    }

    // ----
    inline int retIf(cc cond)
    {
        if (m_registers->getCondition(cond))
        {
            ret();

            return 5;
        }

        return 2;
    }

    // ----
    inline u8 pop()
    {
        auto ret{m_memoryPtr->get(m_registers->getSP())};
        m_registers->incrementSP();
        return ret;
    }

    // ----
    inline u16 pop16()
    {
        auto ret{m_memoryPtr->get16(m_registers->getSP())};
        m_registers->incrementSP(2);
        return ret;
    }

    // ----
    inline int push16(u16 val)
    {
        m_registers->decrementSP(2);
        m_memoryPtr->set16(m_registers->getSP(), val);

        return -1;
    }

    // ----
    inline int pushRegister16(r16 reg)
    {
        push16(m_registers->get16(reg));

        return -1;
    }

    // ----
    inline int call(u16 addr)
    {
        push16(m_registers->getPC()+m_opcodeSize);
        jpToAddress(addr);

        return 6;
    }

    // ----
    inline int callIf(cc cond, u16 addr)
    {
        if (m_registers->getCondition(cond))
        {
            call(addr);

            return 6;
        }

        return 3;
    }

    // ----
    inline int callVector(vec addr)
    {
        call(addr);

        return 4;
    }

    // ----
    inline int disableInterrupts()
    {
        m_registers->unsetIme();

        return 1;
    }

    // ----
    inline void enableInterrupts()
    {
        m_registers->setIme();
    }

    // Z-0C
    inline int decimalAdjustAccumulator()
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

        return 1;
    }
    
    inline void ILLEGAL_INSTRUCTION(opcode_t opcode)
    {
        // The Z80-like processors do not crash the system when
        // encountering an illegal instruction, so just report it.
        Logger::warning("Illegal instruction: " + toHexStr(opcode));

        std::string message{
                "Invalid opcode: " + toHexStr(opcode) + "\n" +
                "PC: " + toHexStr(m_registers->getPC()) + "\n" +
                "SP: " + toHexStr(m_registers->getSP()) + "\n" +
                "\n"};

        for (int i{-8}; i <= 8; ++i)
        {
            if (i == 0) message += ">";
            message += toHexStr(m_memoryPtr->get(m_registers->getPC() + i), 2, false);
            if (i == 0) message += "<";
            if (i != 8) message += " ";
        }

        message +=
                std::string("\n") +
                "\nThis is probably a bug in the ROM or in the emulator";

        SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_WARNING,
                "Invalid Opcode",
                message.c_str(), nullptr);
    }

    //=========================================================================

    inline int i_0x00()        { return 1; }
    inline int i_0x01(u16 x)   { return setRegister16(r16::BC, x); }
    inline int i_0x02()        { return setValueAtAddressInRegister16ToRegister8(r16::BC, r8::A); }
    inline int i_0x03()        { return incrementRegister16(r16::BC); }
    inline int i_0x04()        { return incrementRegister8F(r8::B); }
    inline int i_0x05()        { return decrementRegister8F(r8::B); }
    inline int i_0x06(u8 x)    { return setRegister8(r8::B, x); }
    inline int i_0x07()        { return rotateRegister8BitsLeftF(r8::A); }
    inline int i_0x08(u16 x)   { m_memoryPtr->set16(x, m_registers->getSP()); return 5; }
    inline int i_0x09()        { return addRegister16ToRegister16F(r16::HL, r16::BC); }
    inline int i_0x0a()        { return setRegister8ToValueAtAddressInRegister16(r8::A, r16::BC); }
    inline int i_0x0b()        { return decrementRegister16(r16::BC); }
    inline int i_0x0c()        { return incrementRegister8F(r8::C); }
    inline int i_0x0d()        { return decrementRegister8F(r8::C); }
    inline int i_0x0e(u8 x)    { return setRegister8(r8::C, x); }
    inline int i_0x0f()        { return rotateARegBitsRightF(); }
    inline int i_0x10()        { UNIMPLEMENTED(); return 1; }
    inline int i_0x11(u16 x)   { return setRegister16(r16::DE, x); }
    inline int i_0x12()        { return setValueAtAddressInRegister16ToRegister8(r16::DE, r8::A); }
    inline int i_0x13()        { return incrementRegister16(r16::DE); }
    inline int i_0x14()        { return incrementRegister8F(r8::D); }
    inline int i_0x15()        { return decrementRegister8F(r8::D); }
    inline int i_0x16(u8 x)    { return setRegister8(r8::D, x); }
    inline int i_0x17()        { return rotateARegBitsLeftThroughCarryFlagF(); }
    inline int i_0x18(i8 x)    { return relativeJump(x); }
    inline int i_0x19()        { return addRegister16ToRegister16F(r16::HL, r16::DE); }
    inline int i_0x1a()        { return setRegister8ToValueAtAddressInRegister16(r8::A, r16::DE); }
    inline int i_0x1b()        { return decrementRegister16(r16::DE); }
    inline int i_0x1c()        { return incrementRegister8F(r8::E); }
    inline int i_0x1d()        { return decrementRegister8F(r8::E); }
    inline int i_0x1e(u8 x)    { return setRegister8(r8::E, x); }
    inline int i_0x1f()        { return rotateARegBitsRightThroughCarryFlagF(); }
    inline int i_0x20(i8 x)    { return relativeJumpIf(cc::NZ, x); }
    inline int i_0x21(u16 x)   { return setRegister16(r16::HL, x); }
    inline int i_0x22()        { setValueAtAddressInRegister16(r16::HL, m_registers->getA()); incrementRegister16(r16::HL); return 2; }
    inline int i_0x23()        { return incrementRegister16(r16::HL); }
    inline int i_0x24()        { return incrementRegister8F(r8::H); }
    inline int i_0x25()        { return decrementRegister8F(r8::H); }
    inline int i_0x26(u8 x)    { return setRegister8(r8::H, x); }
    inline int i_0x27()        { return decimalAdjustAccumulator(); }
    inline int i_0x28(i8 x)    { return relativeJumpIf(cc::Z, x); }
    inline int i_0x29()        { return addRegister16ToRegister16F(r16::HL, r16::HL); }
    inline int i_0x2a()        { setRegister8ToValueAtAddressInRegister16(r8::A, r16::HL); incrementRegister16(r16::HL); return 2; }
    inline int i_0x2b()        { return decrementRegister16(r16::HL); }
    inline int i_0x2c()        { return incrementRegister8F(r8::L); }
    inline int i_0x2d()        { return decrementRegister8F(r8::L); }
    inline int i_0x2e(u8 x)    { return setRegister8(r8::L, x); }
    inline int i_0x2f()        { return complementRegister8F(r8::A); }
    inline int i_0x30(i8 x)    { return relativeJumpIf(cc::NC, x); }
    inline int i_0x31(u16 x)   { return setRegister16(r16::SP, x); }
    inline int i_0x32()        { setValueAtAddressInRegister16ToRegister8(r16::HL, r8::A); decrementRegister16(r16::HL); return 2; }
    inline int i_0x33()        { return incrementRegister16(r16::SP); }
    inline int i_0x34()        { return incrementValueAtAddressInRegister16F(r16::HL); }
    inline int i_0x35()        { return decrementValueAtAddressInRegister16F(r16::HL); }
    inline int i_0x36(u8 x)    { return setValueAtAddressInRegister16(r16::HL, x); }
    inline int i_0x37()        { m_registers->unsetNegativeFlag(); m_registers->unsetHalfCarryFlag(); m_registers->setCarryFlag(); return 1; }
    inline int i_0x38(i8 x)    { return relativeJumpIf(cc::C, x); }
    inline int i_0x39()        { return addRegister16ToRegister16F(r16::HL, r16::SP); }
    inline int i_0x3a()        { setRegister8ToValueAtAddressInRegister16(r8::A, r16::HL); decrementRegister16(r16::HL); return 2; }
    inline int i_0x3b()        { return decrementRegister16(r16::SP); }
    inline int i_0x3c()        { return incrementRegister8F(r8::A); }
    inline int i_0x3d()        { return decrementRegister8F(r8::A); }
    inline int i_0x3e(u8 x)    { return setRegister8(r8::A, x); }
    inline int i_0x3f()        { m_registers->unsetNegativeFlag(); m_registers->unsetHalfCarryFlag(); m_registers->setCarryFlag(!m_registers->getCarryFlag()); return 1; }
    inline int i_0x40()        { return setRegister8ToRegister8(r8::B, r8::B); }
    inline int i_0x41()        { return setRegister8ToRegister8(r8::B, r8::C); }
    inline int i_0x42()        { return setRegister8ToRegister8(r8::B, r8::D); }
    inline int i_0x43()        { return setRegister8ToRegister8(r8::B, r8::E); }
    inline int i_0x44()        { return setRegister8ToRegister8(r8::B, r8::H); }
    inline int i_0x45()        { return setRegister8ToRegister8(r8::B, r8::L); }
    inline int i_0x46()        { return setRegister8ToValueAtAddressInRegister16(r8::B, r16::HL); }
    inline int i_0x47()        { return setRegister8ToRegister8(r8::B, r8::A); }
    inline int i_0x48()        { return setRegister8ToRegister8(r8::C, r8::B); }
    inline int i_0x49()        { return setRegister8ToRegister8(r8::C, r8::C); }
    inline int i_0x4a()        { return setRegister8ToRegister8(r8::C, r8::D); }
    inline int i_0x4b()        { return setRegister8ToRegister8(r8::C, r8::E); }
    inline int i_0x4c()        { return setRegister8ToRegister8(r8::C, r8::H); }
    inline int i_0x4d()        { return setRegister8ToRegister8(r8::C, r8::L); }
    inline int i_0x4e()        { return setRegister8ToValueAtAddressInRegister16(r8::C, r16::HL); }
    inline int i_0x4f()        { return setRegister8ToRegister8(r8::C, r8::A); }
    inline int i_0x50()        { return setRegister8ToRegister8(r8::D, r8::B); }
    inline int i_0x51()        { return setRegister8ToRegister8(r8::D, r8::C); }
    inline int i_0x52()        { return setRegister8ToRegister8(r8::D, r8::D); }
    inline int i_0x53()        { return setRegister8ToRegister8(r8::D, r8::E); }
    inline int i_0x54()        { return setRegister8ToRegister8(r8::D, r8::H); }
    inline int i_0x55()        { return setRegister8ToRegister8(r8::D, r8::L); }
    inline int i_0x56()        { return setRegister8ToValueAtAddressInRegister16(r8::D, r16::HL); }
    inline int i_0x57()        { return setRegister8ToRegister8(r8::D, r8::A); }
    inline int i_0x58()        { return setRegister8ToRegister8(r8::E, r8::B); }
    inline int i_0x59()        { return setRegister8ToRegister8(r8::E, r8::C); }
    inline int i_0x5a()        { return setRegister8ToRegister8(r8::E, r8::D); }
    inline int i_0x5b()        { return setRegister8ToRegister8(r8::E, r8::E); }
    inline int i_0x5c()        { return setRegister8ToRegister8(r8::E, r8::H); }
    inline int i_0x5d()        { return setRegister8ToRegister8(r8::E, r8::L); }
    inline int i_0x5e()        { return setRegister8ToValueAtAddressInRegister16(r8::E, r16::HL); }
    inline int i_0x5f()        { return setRegister8ToRegister8(r8::E, r8::A); }
    inline int i_0x60()        { return setRegister8ToRegister8(r8::H, r8::B); }
    inline int i_0x61()        { return setRegister8ToRegister8(r8::H, r8::C); }
    inline int i_0x62()        { return setRegister8ToRegister8(r8::H, r8::D); }
    inline int i_0x63()        { return setRegister8ToRegister8(r8::H, r8::E); }
    inline int i_0x64()        { return setRegister8ToRegister8(r8::H, r8::H); }
    inline int i_0x65()        { return setRegister8ToRegister8(r8::H, r8::L); }
    inline int i_0x66()        { return setRegister8ToValueAtAddressInRegister16(r8::H, r16::HL); }
    inline int i_0x67()        { return setRegister8ToRegister8(r8::H, r8::A); }
    inline int i_0x68()        { return setRegister8ToRegister8(r8::L, r8::B); }
    inline int i_0x69()        { return setRegister8ToRegister8(r8::L, r8::C); }
    inline int i_0x6a()        { return setRegister8ToRegister8(r8::L, r8::D); }
    inline int i_0x6b()        { return setRegister8ToRegister8(r8::L, r8::E); }
    inline int i_0x6c()        { return setRegister8ToRegister8(r8::L, r8::H); }
    inline int i_0x6d()        { return setRegister8ToRegister8(r8::L, r8::L); }
    inline int i_0x6e()        { return setRegister8ToValueAtAddressInRegister16(r8::L, r16::HL); }
    inline int i_0x6f()        { return setRegister8ToRegister8(r8::L, r8::A); }
    inline int i_0x70()        { return setValueAtAddressInRegister16ToRegister8(r16::HL, r8::B); }
    inline int i_0x71()        { return setValueAtAddressInRegister16ToRegister8(r16::HL, r8::C); }
    inline int i_0x72()        { return setValueAtAddressInRegister16ToRegister8(r16::HL, r8::D); }
    inline int i_0x73()        { return setValueAtAddressInRegister16ToRegister8(r16::HL, r8::E); }
    inline int i_0x74()        { return setValueAtAddressInRegister16ToRegister8(r16::HL, r8::H); }
    inline int i_0x75()        { return setValueAtAddressInRegister16ToRegister8(r16::HL, r8::L); }
    inline int i_0x76()        { UNIMPLEMENTED(); return 1; }
    inline int i_0x77()        { return setValueAtAddressInRegister16ToRegister8(r16::HL, r8::A); }
    inline int i_0x78()        { return setRegister8ToRegister8(r8::A, r8::B); }
    inline int i_0x79()        { return setRegister8ToRegister8(r8::A, r8::C); }
    inline int i_0x7a()        { return setRegister8ToRegister8(r8::A, r8::D); }
    inline int i_0x7b()        { return setRegister8ToRegister8(r8::A, r8::E); }
    inline int i_0x7c()        { return setRegister8ToRegister8(r8::A, r8::H); }
    inline int i_0x7d()        { return setRegister8ToRegister8(r8::A, r8::L); }
    inline int i_0x7e()        { return setRegister8ToValueAtAddressInRegister16(r8::A, r16::HL); }
    inline int i_0x7f()        { return setRegister8ToRegister8(r8::A, r8::A); }
    inline int i_0x80()        { return addRegister8ToRegister8F(r8::A, r8::B); }
    inline int i_0x81()        { return addRegister8ToRegister8F(r8::A, r8::C); }
    inline int i_0x82()        { return addRegister8ToRegister8F(r8::A, r8::D); }
    inline int i_0x83()        { return addRegister8ToRegister8F(r8::A, r8::E); }
    inline int i_0x84()        { return addRegister8ToRegister8F(r8::A, r8::H); }
    inline int i_0x85()        { return addRegister8ToRegister8F(r8::A, r8::L); }
    inline int i_0x86()        { return addValueAtAddressInRegister16ToRegister8F(r8::A, r16::HL); }
    inline int i_0x87()        { return addRegister8ToRegister8F(r8::A, r8::A); }
    inline int i_0x88()        { return addRegister8AndCarryFlagToRegister8F(r8::A, r8::B); }
    inline int i_0x89()        { return addRegister8AndCarryFlagToRegister8F(r8::A, r8::C); }
    inline int i_0x8a()        { return addRegister8AndCarryFlagToRegister8F(r8::A, r8::D); }
    inline int i_0x8b()        { return addRegister8AndCarryFlagToRegister8F(r8::A, r8::E); }
    inline int i_0x8c()        { return addRegister8AndCarryFlagToRegister8F(r8::A, r8::H); }
    inline int i_0x8d()        { return addRegister8AndCarryFlagToRegister8F(r8::A, r8::L); }
    inline int i_0x8e()        { return addValueAndCarryFlagToRegister8F(r8::A, m_memoryPtr->get(m_registers->getHL())); }
    inline int i_0x8f()        { return addRegister8AndCarryFlagToRegister8F(r8::A, r8::A); }
    inline int i_0x90()        { return subRegister8FromRegister8F(r8::A, r8::B); }
    inline int i_0x91()        { return subRegister8FromRegister8F(r8::A, r8::C); }
    inline int i_0x92()        { return subRegister8FromRegister8F(r8::A, r8::D); }
    inline int i_0x93()        { return subRegister8FromRegister8F(r8::A, r8::E); }
    inline int i_0x94()        { return subRegister8FromRegister8F(r8::A, r8::H); }
    inline int i_0x95()        { return subRegister8FromRegister8F(r8::A, r8::L); }
    inline int i_0x96()        { return subFromRegister8F(r8::A, m_memoryPtr->get(m_registers->getHL())); }
    inline int i_0x97()        { return subRegister8FromRegister8F(r8::A, r8::A); }
    inline int i_0x98()        { return subRegister8AndCarryFlagFromRegister8F(r8::A, r8::B); }
    inline int i_0x99()        { return subRegister8AndCarryFlagFromRegister8F(r8::A, r8::C); }
    inline int i_0x9a()        { return subRegister8AndCarryFlagFromRegister8F(r8::A, r8::D); }
    inline int i_0x9b()        { return subRegister8AndCarryFlagFromRegister8F(r8::A, r8::E); }
    inline int i_0x9c()        { return subRegister8AndCarryFlagFromRegister8F(r8::A, r8::H); }
    inline int i_0x9d()        { return subRegister8AndCarryFlagFromRegister8F(r8::A, r8::L); }
    inline int i_0x9e()        { return subValueAndCarryFlagFromRegister8F(r8::A, m_memoryPtr->get(m_registers->getHL())); }
    inline int i_0x9f()        { return subRegister8AndCarryFlagFromRegister8F(r8::A, r8::A); }
    inline int i_0xa0()        { return andRegister8AndRegister8F(r8::A, r8::B); }
    inline int i_0xa1()        { return andRegister8AndRegister8F(r8::A, r8::C); }
    inline int i_0xa2()        { return andRegister8AndRegister8F(r8::A, r8::D); }
    inline int i_0xa3()        { return andRegister8AndRegister8F(r8::A, r8::E); }
    inline int i_0xa4()        { return andRegister8AndRegister8F(r8::A, r8::H); }
    inline int i_0xa5()        { return andRegister8AndRegister8F(r8::A, r8::L); }
    inline int i_0xa6()        { return andValueAndRegister8F(r8::A, m_memoryPtr->get(m_registers->getHL())); }
    inline int i_0xa7()        { return andRegister8AndRegister8F(r8::A, r8::A); }
    inline int i_0xa8()        { return xorRegister8AndRegister8F(r8::A, r8::B); }
    inline int i_0xa9()        { return xorRegister8AndRegister8F(r8::A, r8::C); }
    inline int i_0xaa()        { return xorRegister8AndRegister8F(r8::A, r8::D); }
    inline int i_0xab()        { return xorRegister8AndRegister8F(r8::A, r8::E); }
    inline int i_0xac()        { return xorRegister8AndRegister8F(r8::A, r8::H); }
    inline int i_0xad()        { return xorRegister8AndRegister8F(r8::A, r8::L); }
    inline int i_0xae()        { return xorValueAndRegister8F(r8::A, m_memoryPtr->get(m_registers->getHL())); }
    inline int i_0xaf()        { return xorRegister8AndRegister8F(r8::A, r8::A); }
    inline int i_0xb0()        { return orRegister8AndRegister8F(r8::A, r8::B); }
    inline int i_0xb1()        { return orRegister8AndRegister8F(r8::A, r8::C); }
    inline int i_0xb2()        { return orRegister8AndRegister8F(r8::A, r8::D); }
    inline int i_0xb3()        { return orRegister8AndRegister8F(r8::A, r8::E); }
    inline int i_0xb4()        { return orRegister8AndRegister8F(r8::A, r8::H); }
    inline int i_0xb5()        { return orRegister8AndRegister8F(r8::A, r8::L); }
    inline int i_0xb6()        { return orValueAndRegister8F(r8::A, m_memoryPtr->get(m_registers->getHL()));}
    inline int i_0xb7()        { return orRegister8AndRegister8F(r8::A, r8::A); }
    inline int i_0xb8()        { return cpRegister8AndRegister8F(r8::A, r8::B); }
    inline int i_0xb9()        { return cpRegister8AndRegister8F(r8::A, r8::C); }
    inline int i_0xba()        { return cpRegister8AndRegister8F(r8::A, r8::D); }
    inline int i_0xbb()        { return cpRegister8AndRegister8F(r8::A, r8::E); }
    inline int i_0xbc()        { return cpRegister8AndRegister8F(r8::A, r8::H); }
    inline int i_0xbd()        { return cpRegister8AndRegister8F(r8::A, r8::L); }
    inline int i_0xbe()        { return cpRegister8AndValue(r8::A, m_memoryPtr->get(m_registers->getHL())); }
    inline int i_0xbf()        { return cpRegister8AndRegister8F(r8::A, r8::A); }
    inline int i_0xc0()        { return retIf(cc::NZ); }
    inline int i_0xc1()        { return pop(); }
    inline int i_0xc2(u16 x)   { return jpIf(cc::NZ, x); }
    inline int i_0xc3(u16 x)   { return jpToAddress(x); }
    inline int i_0xc4(u16 x)   { return callIf(cc::NZ, x); }
    inline int i_0xc5()        { return pushRegister16(r16::BC); }
    inline int i_0xc6(u8 x)    { return addToRegister8F(r8::A, x); }
    inline int i_0xc7()        { return callVector(JUMP_VECTOR_00); }
    inline int i_0xc8()        { return retIf(cc::Z); }
    inline int i_0xc9()        { return ret(); }
    inline int i_0xca(u16 x)   { return jpIf(cc::Z, x); }
    inline int i_0xcb()        { UNIMPLEMENTED(); Logger::error("Found an CB prefix"); return 1; }
    inline int i_0xcc(u16 x)   { return callIf(cc::Z, x); }
    inline int i_0xcd(u16 x)   { return call(x); }
    inline int i_0xce(u8 x)    { return addValueAndCarryFlagToRegister8F(r8::A, x); }
    inline int i_0xcf()        { return callVector(JUMP_VECTOR_08); }
    inline int i_0xd0()        { return retIf(cc::NC); }
    inline int i_0xd1()        { m_registers->set16(r16::DE, pop16()); return 3; }
    inline int i_0xd2(u16 x)   { return jpIf(cc::NC, x); }
    inline int i_0xd3()        { ILLEGAL_INSTRUCTION(0xd3); return 0; }
    inline int i_0xd4(u16 x)   { return callIf(cc::NC, x); }
    inline int i_0xd5()        { return pushRegister16(r16::DE); }
    inline int i_0xd6(u8 x)    { return subFromRegister8F(r8::A, x); }
    inline int i_0xd7()        { return callVector(JUMP_VECTOR_10); }
    inline int i_0xd8()        { return retIf(cc::C); }
    inline int i_0xd9()        { enableInterrupts(); ret(); return 4; }
    inline int i_0xda(u16 x)   { return jpIf(cc::C, x); }
    inline int i_0xdb()        { ILLEGAL_INSTRUCTION(0xdb); return 0; }
    inline int i_0xdc(u16 x)   { return callIf(cc::C, x); }
    inline int i_0xdd()        { ILLEGAL_INSTRUCTION(0xdd); return 0; }
    inline int i_0xde(u8 x)    { return subValueAndCarryFlagFromRegister8F(r8::A, x); }
    inline int i_0xdf()        { return callVector(JUMP_VECTOR_18); }
    inline int i_0xe0(u8 x)    { setValueAtAddressToRegister8(0xff00+x, r8::A); return 3; }
    inline int i_0xe1()        { m_registers->setHL(pop16()); return 3; }
    inline int i_0xe2()        { setValueAtAddressToRegister8(0xff00+m_registers->getC(), r8::A); return 2; }
    inline int i_0xe3()        { ILLEGAL_INSTRUCTION(0xe3); return 0; }
    inline int i_0xe4()        { ILLEGAL_INSTRUCTION(0xe4); return 0;}
    inline int i_0xe5()        { return pushRegister16(r16::HL); }
    inline int i_0xe6(u8 x)    { return andValueAndRegister8F(r8::A, x); }
    inline int i_0xe7()        { return callVector(JUMP_VECTOR_20); }
    inline int i_0xe8(i8 x)    { m_registers->incrementSP(x); return 4; }
    inline int i_0xe9()        { return jpToAddressInRegister16(r16::HL); }
    inline int i_0xea(u8 x)    { return setValueAtAddressToRegister8(x, r8::A); }
    inline int i_0xeb()        { ILLEGAL_INSTRUCTION(0xeb); return 0; }
    inline int i_0xec()        { ILLEGAL_INSTRUCTION(0xec); return 0; }
    inline int i_0xed()        { ILLEGAL_INSTRUCTION(0xed); return 0;}
    inline int i_0xee(u8 x)    { return xorValueAndRegister8F(r8::A, x); }
    inline int i_0xef()        { return callVector(JUMP_VECTOR_28); }
    inline int i_0xf0(u8 x)    { setRegister8(r8::A, m_memoryPtr->get(0xff00+x)); return 3; }
    inline int i_0xf1()        { m_registers->setAF(pop16()); return 3; }
    inline int i_0xf2()        { setRegister8(r8::A, m_memoryPtr->get(0xff00+m_registers->getC())); return 2; }
    inline int i_0xf3()        { return disableInterrupts(); }
    inline int i_0xf4()        { ILLEGAL_INSTRUCTION(0xf4); return 0; }
    inline int i_0xf5()        { return pushRegister16(r16::AF); }
    inline int i_0xf6(u8 x)    { return orValueAndRegister8F(r8::A, x); }
    inline int i_0xf7()        { return callVector(JUMP_VECTOR_30); }
    inline int i_0xf8(i8 x)    { setRegister16(r16::HL, m_registers->getSP()+x); return 3; } // TODO: Set flags
    inline int i_0xf9()        { return setRegister16ToRegister16(r16::SP, r16::HL); }
    inline int i_0xfa(u8 x)    { return setRegister8(r8::A, x); }
    inline int i_0xfb()        { m_wasEiInstruction = true; return 1; }
    inline int i_0xfc()        { ILLEGAL_INSTRUCTION(0xfc); return 0; }
    inline int i_0xfd()        { ILLEGAL_INSTRUCTION(0xfd); return 0; }
    inline int i_0xfe(u8 x)    { return cpRegister8AndValue(r8::A, x); }
    inline int i_0xff()        { return callVector(JUMP_VECTOR_38); }
};



#endif /* CPU_H_ */
