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

    bool            m_isPrefixedOpcode{};

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
    int emulateCurrentPrefixedOpcode();

    inline bool isPrefixedOpcode() const { return m_isPrefixedOpcode; }

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
    inline int addToARegF(u8 value)
    {
        m_registers->setHalfCarryFlag(wouldAddHalfCarry8(m_registers->getA(), value));
        m_registers->setCarryFlag(wouldAddCarry8(m_registers->getA(), value));
        m_registers->setA(m_registers->getA()+value);
        m_registers->setZeroFlag(m_registers->getA() == 0);
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
    inline int subFromARegF(u8 value)
    {
        m_registers->setHalfCarryFlag(wouldSubHalfCarry8(m_registers->getA(), value));
        m_registers->setCarryFlag(wouldSubCarry8(m_registers->getA(), value));
        m_registers->setA(m_registers->getA()-value);
        m_registers->setZeroFlag(m_registers->getA() == 0);
        m_registers->unsetNegativeFlag();

        return 2;
    }

    // -0HC
    inline int addRegister16ToHLRegF(r16 src)
    {
        m_registers->setHalfCarryFlag(wouldAddHalfCarry16(m_registers->getHL(), m_registers->get16(src)));
        m_registers->setCarryFlag(wouldAddCarry16(m_registers->getHL(), m_registers->get16(src)));
        m_registers->setHL(m_registers->getHL()+m_registers->get16(src));
        m_registers->unsetNegativeFlag();

        return 2;
    }

    // ----
    inline int setValueAtAddressInHLReg(u8 value)
    {
        m_memoryPtr->set(m_registers->getHL(), value);

        return 3;
    }

    // ----
    inline int setValueAtAddressInRegister16ToRegister8(r16 addr, r8 val)
    {
        m_memoryPtr->set(m_registers->get16(addr), m_registers->get8(val));

        return 2;
    }

    // ----
    inline int setValueAtAddressToAReg(u16 addr)
    {
        m_memoryPtr->set(addr, m_registers->getA());

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
    inline int incrementValueAtAddressInHLReg()
    {
        m_registers->setHalfCarryFlag(wouldAddHalfCarry8(m_memoryPtr->get(m_registers->getHL()), 1));
        m_memoryPtr->set(m_registers->getHL(), m_memoryPtr->get(m_registers->getHL())+1);
        m_registers->setZeroFlag(m_memoryPtr->get(m_registers->getHL()) == 0);
        m_registers->unsetNegativeFlag();

        return 3;
    }

    // Z1H-
    inline int decrementValueAtAddressInHLReg()
    {
        m_registers->setHalfCarryFlag(wouldSubHalfCarry8(m_memoryPtr->get(m_registers->getHL()), 1));
        m_memoryPtr->set(m_registers->getHL(), m_memoryPtr->get(m_registers->getHL())-1);
        m_registers->setZeroFlag(m_memoryPtr->get(m_registers->getHL()) == 0);
        m_registers->setNegativeFlag();

        return 3;
    }

    // Z0HC
    inline int addRegister8ToARegF(r8 src)
    {
        m_registers->setHalfCarryFlag(wouldAddHalfCarry8(m_registers->getA(), m_registers->get8(src)));
        m_registers->setCarryFlag(wouldAddCarry8(m_registers->getA(), m_registers->get8(src)));
        m_registers->setA(m_registers->getA()+m_registers->get8(src));
        m_registers->setZeroFlag(m_registers->getA() == 0);
        m_registers->unsetNegativeFlag();

        return 1;
    }

    // Z0HC
    inline int addValueAtAddressInHLRegToARegF()
    {
        auto value{m_memoryPtr->get(m_registers->getHL())};
        m_registers->setHalfCarryFlag(wouldAddHalfCarry8(m_registers->getA(), value));
        m_registers->setCarryFlag(wouldAddCarry8(m_registers->getA(), value));
        m_registers->setA(m_registers->getA()+value);
        m_registers->setZeroFlag(m_registers->getA() == 0);
        m_registers->unsetNegativeFlag();

        return 2;
    }

    // Z0HC
    inline int addRegister8AndCarryFlagToARegF(r8 src)
    {
        m_registers->setHalfCarryFlag(wouldAddHalfCarry8(m_registers->getA(), m_registers->get8(src)+m_registers->getCarryFlag()));
        m_registers->setCarryFlag(wouldAddCarry8(m_registers->getA(), m_registers->get8(src)+m_registers->getCarryFlag()));
        m_registers->setA(m_registers->getA()+m_registers->get8(src)+m_registers->getCarryFlag());
        m_registers->setZeroFlag(m_registers->getA() == 0);
        m_registers->unsetNegativeFlag();

        return 2;
    }

    // Z1HC
    inline int subRegister8FromARegF(r8 src)
    {
        m_registers->setHalfCarryFlag(wouldSubHalfCarry8(m_registers->getA(), m_registers->get8(src)));
        m_registers->setCarryFlag(wouldSubCarry8(m_registers->getA(), m_registers->get8(src)));
        m_registers->setA(m_registers->getA()-m_registers->get8(src));
        m_registers->setZeroFlag(m_registers->getA() == 0);
        m_registers->setNegativeFlag();

        return 1;
    }

    // Z1HC
    inline int subRegister8AndCarryFlagFromARegF(r8 src)
    {
        m_registers->setHalfCarryFlag(wouldSubHalfCarry8(m_registers->getA(), m_registers->get8(src)+m_registers->getCarryFlag()));
        m_registers->setCarryFlag(wouldSubCarry8(m_registers->getA(), m_registers->get8(src)+m_registers->getCarryFlag()));
        m_registers->setA(m_registers->getA()-m_registers->get8(src)-m_registers->getCarryFlag());
        m_registers->setZeroFlag(m_registers->getA() == 0);
        m_registers->setNegativeFlag();

        return 1;
    }

    // Z010
    inline int andRegister8AndARegF(r8 src)
    {
        m_registers->setA(m_registers->getA() & m_registers->get8(src));
        m_registers->setZeroFlag(m_registers->getA() == 0);
        m_registers->unsetNegativeFlag();
        m_registers->setHalfCarryFlag();
        m_registers->unsetCarryFlag();

        return 1;
    }

    // Z000
    inline int xorRegister8AndARegF(r8 src)
    {
        m_registers->setA(m_registers->getA() ^ m_registers->get8(src));
        m_registers->setZeroFlag(m_registers->getA() == 0);
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();
        m_registers->unsetCarryFlag();

        return 1;
    }

    // Z000
    inline int orRegister8AndARegF(r8 src)
    {
        m_registers->setA(m_registers->getA() | m_registers->get8(src));
        m_registers->setZeroFlag(m_registers->getA() == 0);
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();
        m_registers->unsetCarryFlag();

        return 1;
    }

    // Z1HC
    inline int cpARegAndRegister8F(r8 reg2)
    {
        m_registers->setZeroFlag(m_registers->getA() == m_registers->get8(reg2));
        m_registers->setNegativeFlag();
        m_registers->setHalfCarryFlag(wouldSubHalfCarry8(m_registers->getA(), m_registers->get8(reg2)));
        m_registers->setCarryFlag(wouldSubCarry8(m_registers->getA(), m_registers->get8(reg2)));

        return 1;
    }

    // Z0HC
    inline int addValueAndCarryFlagToARegF(u8 val)
    {
        m_registers->setHalfCarryFlag(wouldAddHalfCarry8(m_registers->getA(), val+m_registers->getCarryFlag()));
        m_registers->setCarryFlag(wouldAddCarry8(m_registers->getA(), val+m_registers->getCarryFlag()));
        m_registers->setA(m_registers->getA()+val+m_registers->getCarryFlag());
        m_registers->setZeroFlag(m_registers->getA() == 0);
        m_registers->unsetNegativeFlag();

        return 2;
    }

    // Z1HC
    inline int subValueAndCarryFlagFromARegF(u8 val)
    {
        m_registers->setHalfCarryFlag(wouldSubHalfCarry8(m_registers->getA(), val+m_registers->getCarryFlag()));
        m_registers->setCarryFlag(wouldSubCarry8(m_registers->getA(), val+m_registers->getCarryFlag()));
        m_registers->setA(m_registers->getA()-val-m_registers->getCarryFlag());
        m_registers->setZeroFlag(m_registers->getA() == 0);
        m_registers->setNegativeFlag();

        return 2;
    }

    // Z010
    inline int andValueAndARegF(u8 val)
    {
        m_registers->setA(m_registers->getA() & val);
        m_registers->setZeroFlag(m_registers->getA() == 0);
        m_registers->unsetNegativeFlag();
        m_registers->setHalfCarryFlag();
        m_registers->unsetCarryFlag();

        return 2;
    }

    // Z000
    inline int xorValueAndARegF(u8 val)
    {
        m_registers->setA(m_registers->getA() ^ val);
        m_registers->setZeroFlag(m_registers->getA() == 0);
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();
        m_registers->unsetCarryFlag();

        return 2;
    }

    // Z000
    inline int orValueAndARegF(u8 val)
    {
        m_registers->setA(m_registers->getA() | val);
        m_registers->setZeroFlag(m_registers->getA() == 0);
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();
        m_registers->unsetCarryFlag();

        return 2;
    }

    // Z1HC
    inline int cpARegAndValue(u8 val)
    {
        m_registers->setZeroFlag(m_registers->getA() == val);
        m_registers->setNegativeFlag();
        m_registers->setHalfCarryFlag(wouldSubHalfCarry8(m_registers->getA(), val));
        m_registers->setCarryFlag(wouldSubCarry8(m_registers->getA(), val));

        return 2;
    }

    // 000C
    inline int rotateARegBitsLeftF()
    {
        m_registers->setCarryFlag(m_registers->getA() & 1);
        m_registers->setA((m_registers->getA() << 1) | (m_registers->getA() >> 7));

        m_registers->unsetZeroFlag();
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();

        return 1;
    }

    // 000C
    inline int rotateARegBitsRightF()
    {
        m_registers->setCarryFlag(m_registers->getA() & 1);
        m_registers->setA((m_registers->getA() >> 1) | (m_registers->getA() << 7));

        m_registers->unsetZeroFlag();
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();

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
        // FIXME: Add 2?
        jpToAddress(m_registers->getPC()+offset+2);

        return 3;
    }

    // -11-
    inline int complementARegF()
    {
        m_registers->setA(~m_registers->getA());

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
    inline int jpToAddressInHLReg()
    {
        jpToAddress(m_registers->getHL());

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

    inline int handlePrefix()
    {
        m_isPrefixedOpcode = true;

        return 1;
    }

    // ------------------ prefixed ----------------

    // Z00C
    inline int rotateRegisterBitsLeftF(r8 reg)
    {
        m_registers->set8(reg, m_registers->get8(reg) << 1 | m_registers->get8(reg) >> 7);
        m_registers->setZeroFlag(m_registers->get8(reg) == 0);
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();
        m_registers->setCarryFlag(m_registers->get8(reg) & 1);

        return 2;
    }

    // Z00C
    inline int rotateRegisterBitsRightF(r8 reg)
    {
        m_registers->set8(reg, m_registers->get8(reg) >> 1 | m_registers->get8(reg) << 7);
        m_registers->setZeroFlag(m_registers->get8(reg) == 0);
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();
        m_registers->setCarryFlag(m_registers->get8(reg) & (1 << 7));

        return 2;
    }

    // Z00C
    inline int rotateRegisterBitsLeftThroughCarryF(r8 reg)
    {
        auto regVal{m_registers->get8(reg)};

        m_registers->set8(reg, (regVal << 1) | m_registers->getCarryFlag());
        m_registers->setCarryFlag(regVal >> 7);
        m_registers->setZeroFlag(m_registers->get8(reg) == 0);
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();

        return 2;
    }

    // Z00C
    inline int rotateRegisterBitsRightThroughCarryF(r8 reg)
    {
        auto regVal{m_registers->get8(reg)};

        m_registers->set8(reg, regVal >> 1 | (m_registers->getCarryFlag() << 7));
        m_registers->setCarryFlag(regVal & 1);
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();

        return 2;
    }

    // Z00C
    inline int shiftRegisterBitsLeftToCarryF(r8 reg)
    {
        auto origVal{m_registers->get8(reg)};

        m_registers->setCarryFlag(origVal & (1 << 7));
        m_registers->set8(reg, origVal << 1);
        m_registers->setZeroFlag(m_registers->get8(reg) == 0);
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();

        return 2;
    }

    // Z00C
    inline int shiftRegisterBitsRightToCarryF(r8 reg)
    {
        auto origVal{m_registers->get8(reg)};

        // Note: MSB remains unchanged
        m_registers->set8(reg, (origVal >> 1) | (origVal & (1 << 7)));
        m_registers->setZeroFlag(m_registers->get8(reg) == 0);
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();
        m_registers->setCarryFlag(origVal & 1);

        return 2;
    }

    // Z000
    inline int swapRegisterNibblesF(r8 reg)
    {
        auto origVal{m_registers->get8(reg)};

        m_registers->set8(reg, (origVal << 4) | (origVal >> 4));
        m_registers->setZeroFlag(origVal == 0);
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();
        m_registers->unsetCarryFlag();

        return 2;
    }

    // Z00C
    inline int shiftRightLogicRegisterF(r8 reg)
    {
        auto origVal{m_registers->get8(reg)};

        m_registers->set8(reg, origVal >> 1);
        m_registers->setZeroFlag(m_registers->get8(reg) == 0);
        m_registers->unsetNegativeFlag();
        m_registers->unsetHalfCarryFlag();
        m_registers->setCarryFlag(origVal & 1);

        return 2;
    }

    // Z01-
    inline int checkBitOfRegisterF(u3 bit, r8 reg)
    {
        // Is bit unset?
        m_registers->setZeroFlag((m_registers->get8(reg) & (1 << bit)) == 0);
        m_registers->unsetNegativeFlag();
        m_registers->setHalfCarryFlag();

        return 2;
    }

    // ----
    inline int resetBitOfRegister(u3 bit, r8 reg)
    {
        m_registers->set8(reg, m_registers->get8(reg) & ~(1 << bit));

        return 2;
    }

    // ----
    inline int setBitOfRegister(u3 bit, r8 reg)
    {
        m_registers->set8(reg, m_registers->get8(reg) | (1 << bit));

        return 2;
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
    inline int i_0x07()        { return rotateARegBitsLeftF(); }
    inline int i_0x08(u16 x)   { m_memoryPtr->set16(x, m_registers->getSP()); return 5; }
    inline int i_0x09()        { return addRegister16ToHLRegF(r16::BC); }
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
    inline int i_0x19()        { return addRegister16ToHLRegF(r16::DE); }
    inline int i_0x1a()        { return setRegister8ToValueAtAddressInRegister16(r8::A, r16::DE); }
    inline int i_0x1b()        { return decrementRegister16(r16::DE); }
    inline int i_0x1c()        { return incrementRegister8F(r8::E); }
    inline int i_0x1d()        { return decrementRegister8F(r8::E); }
    inline int i_0x1e(u8 x)    { return setRegister8(r8::E, x); }
    inline int i_0x1f()        { return rotateARegBitsRightThroughCarryFlagF(); }
    inline int i_0x20(i8 x)    { return relativeJumpIf(cc::NZ, x); }
    inline int i_0x21(u16 x)   { return setRegister16(r16::HL, x); }
    inline int i_0x22()        { setValueAtAddressInHLReg(m_registers->getA()); incrementRegister16(r16::HL); return 2; }
    inline int i_0x23()        { return incrementRegister16(r16::HL); }
    inline int i_0x24()        { return incrementRegister8F(r8::H); }
    inline int i_0x25()        { return decrementRegister8F(r8::H); }
    inline int i_0x26(u8 x)    { return setRegister8(r8::H, x); }
    inline int i_0x27()        { return decimalAdjustAccumulator(); }
    inline int i_0x28(i8 x)    { return relativeJumpIf(cc::Z, x); }
    inline int i_0x29()        { return addRegister16ToHLRegF(r16::HL); }
    inline int i_0x2a()        { setRegister8ToValueAtAddressInRegister16(r8::A, r16::HL); incrementRegister16(r16::HL); return 2; }
    inline int i_0x2b()        { return decrementRegister16(r16::HL); }
    inline int i_0x2c()        { return incrementRegister8F(r8::L); }
    inline int i_0x2d()        { return decrementRegister8F(r8::L); }
    inline int i_0x2e(u8 x)    { return setRegister8(r8::L, x); }
    inline int i_0x2f()        { return complementARegF(); }
    inline int i_0x30(i8 x)    { return relativeJumpIf(cc::NC, x); }
    inline int i_0x31(u16 x)   { return setRegister16(r16::SP, x); }
    inline int i_0x32()        { setValueAtAddressInRegister16ToRegister8(r16::HL, r8::A); decrementRegister16(r16::HL); return 2; }
    inline int i_0x33()        { return incrementRegister16(r16::SP); }
    inline int i_0x34()        { return incrementValueAtAddressInHLReg(); }
    inline int i_0x35()        { return decrementValueAtAddressInHLReg(); }
    inline int i_0x36(u8 x)    { return setValueAtAddressInHLReg(x); }
    inline int i_0x37()        { m_registers->unsetNegativeFlag(); m_registers->unsetHalfCarryFlag(); m_registers->setCarryFlag(); return 1; }
    inline int i_0x38(i8 x)    { return relativeJumpIf(cc::C, x); }
    inline int i_0x39()        { return addRegister16ToHLRegF(r16::SP); }
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
    inline int i_0x80()        { return addRegister8ToARegF(r8::B); }
    inline int i_0x81()        { return addRegister8ToARegF(r8::C); }
    inline int i_0x82()        { return addRegister8ToARegF(r8::D); }
    inline int i_0x83()        { return addRegister8ToARegF(r8::E); }
    inline int i_0x84()        { return addRegister8ToARegF(r8::H); }
    inline int i_0x85()        { return addRegister8ToARegF(r8::L); }
    inline int i_0x86()        { return addValueAtAddressInHLRegToARegF(); }
    inline int i_0x87()        { return addRegister8ToARegF(r8::A); }
    inline int i_0x88()        { return addRegister8AndCarryFlagToARegF(r8::B); }
    inline int i_0x89()        { return addRegister8AndCarryFlagToARegF(r8::C); }
    inline int i_0x8a()        { return addRegister8AndCarryFlagToARegF(r8::D); }
    inline int i_0x8b()        { return addRegister8AndCarryFlagToARegF(r8::E); }
    inline int i_0x8c()        { return addRegister8AndCarryFlagToARegF(r8::H); }
    inline int i_0x8d()        { return addRegister8AndCarryFlagToARegF(r8::L); }
    inline int i_0x8e()        { return addValueAndCarryFlagToARegF(m_memoryPtr->get(m_registers->getHL())); }
    inline int i_0x8f()        { return addRegister8AndCarryFlagToARegF(r8::A); }
    inline int i_0x90()        { return subRegister8FromARegF(r8::B); }
    inline int i_0x91()        { return subRegister8FromARegF(r8::C); }
    inline int i_0x92()        { return subRegister8FromARegF(r8::D); }
    inline int i_0x93()        { return subRegister8FromARegF(r8::E); }
    inline int i_0x94()        { return subRegister8FromARegF(r8::H); }
    inline int i_0x95()        { return subRegister8FromARegF(r8::L); }
    inline int i_0x96()        { return subFromARegF(m_memoryPtr->get(m_registers->getHL())); }
    inline int i_0x97()        { return subRegister8FromARegF(r8::A); }
    inline int i_0x98()        { return subRegister8AndCarryFlagFromARegF(r8::B); }
    inline int i_0x99()        { return subRegister8AndCarryFlagFromARegF(r8::C); }
    inline int i_0x9a()        { return subRegister8AndCarryFlagFromARegF(r8::D); }
    inline int i_0x9b()        { return subRegister8AndCarryFlagFromARegF(r8::E); }
    inline int i_0x9c()        { return subRegister8AndCarryFlagFromARegF(r8::H); }
    inline int i_0x9d()        { return subRegister8AndCarryFlagFromARegF(r8::L); }
    inline int i_0x9e()        { return subValueAndCarryFlagFromARegF(m_memoryPtr->get(m_registers->getHL())); }
    inline int i_0x9f()        { return subRegister8AndCarryFlagFromARegF(r8::A); }
    inline int i_0xa0()        { return andRegister8AndARegF(r8::B); }
    inline int i_0xa1()        { return andRegister8AndARegF(r8::C); }
    inline int i_0xa2()        { return andRegister8AndARegF(r8::D); }
    inline int i_0xa3()        { return andRegister8AndARegF(r8::E); }
    inline int i_0xa4()        { return andRegister8AndARegF(r8::H); }
    inline int i_0xa5()        { return andRegister8AndARegF(r8::L); }
    inline int i_0xa6()        { return andValueAndARegF(m_memoryPtr->get(m_registers->getHL())); }
    inline int i_0xa7()        { return andRegister8AndARegF(r8::A); }
    inline int i_0xa8()        { return xorRegister8AndARegF(r8::B); }
    inline int i_0xa9()        { return xorRegister8AndARegF(r8::C); }
    inline int i_0xaa()        { return xorRegister8AndARegF(r8::D); }
    inline int i_0xab()        { return xorRegister8AndARegF(r8::E); }
    inline int i_0xac()        { return xorRegister8AndARegF(r8::H); }
    inline int i_0xad()        { return xorRegister8AndARegF(r8::L); }
    inline int i_0xae()        { return xorValueAndARegF(m_memoryPtr->get(m_registers->getHL())); }
    inline int i_0xaf()        { return xorRegister8AndARegF(r8::A); }
    inline int i_0xb0()        { return orRegister8AndARegF(r8::B); }
    inline int i_0xb1()        { return orRegister8AndARegF(r8::C); }
    inline int i_0xb2()        { return orRegister8AndARegF(r8::D); }
    inline int i_0xb3()        { return orRegister8AndARegF(r8::E); }
    inline int i_0xb4()        { return orRegister8AndARegF(r8::H); }
    inline int i_0xb5()        { return orRegister8AndARegF(r8::L); }
    inline int i_0xb6()        { return orValueAndARegF(m_memoryPtr->get(m_registers->getHL()));}
    inline int i_0xb7()        { return orRegister8AndARegF(r8::A); }
    inline int i_0xb8()        { return cpARegAndRegister8F(r8::B); }
    inline int i_0xb9()        { return cpARegAndRegister8F(r8::C); }
    inline int i_0xba()        { return cpARegAndRegister8F(r8::D); }
    inline int i_0xbb()        { return cpARegAndRegister8F(r8::E); }
    inline int i_0xbc()        { return cpARegAndRegister8F(r8::H); }
    inline int i_0xbd()        { return cpARegAndRegister8F(r8::L); }
    inline int i_0xbe()        { return cpARegAndValue(m_memoryPtr->get(m_registers->getHL())); }
    inline int i_0xbf()        { return cpARegAndRegister8F(r8::A); }
    inline int i_0xc0()        { return retIf(cc::NZ); }
    inline int i_0xc1()        { return pop(); }
    inline int i_0xc2(u16 x)   { return jpIf(cc::NZ, x); }
    inline int i_0xc3(u16 x)   { return jpToAddress(x); }
    inline int i_0xc4(u16 x)   { return callIf(cc::NZ, x); }
    inline int i_0xc5()        { return pushRegister16(r16::BC); }
    inline int i_0xc6(u8 x)    { return addToARegF(x); }
    inline int i_0xc7()        { return callVector(JUMP_VECTOR_00); }
    inline int i_0xc8()        { return retIf(cc::Z); }
    inline int i_0xc9()        { return ret(); }
    inline int i_0xca(u16 x)   { return jpIf(cc::Z, x); }
    inline int i_0xcb()        { return handlePrefix(); }
    inline int i_0xcc(u16 x)   { return callIf(cc::Z, x); }
    inline int i_0xcd(u16 x)   { return call(x); }
    inline int i_0xce(u8 x)    { return addValueAndCarryFlagToARegF(x); }
    inline int i_0xcf()        { return callVector(JUMP_VECTOR_08); }
    inline int i_0xd0()        { return retIf(cc::NC); }
    inline int i_0xd1()        { m_registers->set16(r16::DE, pop16()); return 3; }
    inline int i_0xd2(u16 x)   { return jpIf(cc::NC, x); }
    inline int i_0xd3()        { ILLEGAL_INSTRUCTION(0xd3); return 0; }
    inline int i_0xd4(u16 x)   { return callIf(cc::NC, x); }
    inline int i_0xd5()        { return pushRegister16(r16::DE); }
    inline int i_0xd6(u8 x)    { return subFromARegF(x); }
    inline int i_0xd7()        { return callVector(JUMP_VECTOR_10); }
    inline int i_0xd8()        { return retIf(cc::C); }
    inline int i_0xd9()        { enableInterrupts(); ret(); return 4; }
    inline int i_0xda(u16 x)   { return jpIf(cc::C, x); }
    inline int i_0xdb()        { ILLEGAL_INSTRUCTION(0xdb); return 0; }
    inline int i_0xdc(u16 x)   { return callIf(cc::C, x); }
    inline int i_0xdd()        { ILLEGAL_INSTRUCTION(0xdd); return 0; }
    inline int i_0xde(u8 x)    { return subValueAndCarryFlagFromARegF(x); }
    inline int i_0xdf()        { return callVector(JUMP_VECTOR_18); }
    inline int i_0xe0(u8 x)    { setValueAtAddressToAReg(0xff00+x); return 3; }
    inline int i_0xe1()        { m_registers->setHL(pop16()); return 3; }
    inline int i_0xe2()        { setValueAtAddressToAReg(0xff00+m_registers->getC()); return 2; }
    inline int i_0xe3()        { ILLEGAL_INSTRUCTION(0xe3); return 0; }
    inline int i_0xe4()        { ILLEGAL_INSTRUCTION(0xe4); return 0;}
    inline int i_0xe5()        { return pushRegister16(r16::HL); }
    inline int i_0xe6(u8 x)    { return andValueAndARegF(x); }
    inline int i_0xe7()        { return callVector(JUMP_VECTOR_20); }
    inline int i_0xe8(i8 x)    { m_registers->incrementSP(x); return 4; }
    inline int i_0xe9()        { return jpToAddressInHLReg(); }
    inline int i_0xea(u8 x)    { return setValueAtAddressToAReg(x); }
    inline int i_0xeb()        { ILLEGAL_INSTRUCTION(0xeb); return 0; }
    inline int i_0xec()        { ILLEGAL_INSTRUCTION(0xec); return 0; }
    inline int i_0xed()        { ILLEGAL_INSTRUCTION(0xed); return 0;}
    inline int i_0xee(u8 x)    { return xorValueAndARegF(x); }
    inline int i_0xef()        { return callVector(JUMP_VECTOR_28); }
    inline int i_0xf0(u8 x)    { setRegister8(r8::A, m_memoryPtr->get(0xff00+x)); return 3; }
    inline int i_0xf1()        { m_registers->setAF(pop16()); return 3; }
    inline int i_0xf2()        { setRegister8(r8::A, m_memoryPtr->get(0xff00+m_registers->getC())); return 2; }
    inline int i_0xf3()        { return disableInterrupts(); }
    inline int i_0xf4()        { ILLEGAL_INSTRUCTION(0xf4); return 0; }
    inline int i_0xf5()        { return pushRegister16(r16::AF); }
    inline int i_0xf6(u8 x)    { return orValueAndARegF(x); }
    inline int i_0xf7()        { return callVector(JUMP_VECTOR_30); }
    inline int i_0xf8(i8 x)    { setRegister16(r16::HL, m_registers->getSP()+x); return 3; } // TODO: Set flags
    inline int i_0xf9()        { m_registers->setSP(m_registers->getHL()); return 2; }
    inline int i_0xfa(u8 x)    { return setRegister8(r8::A, x); }
    inline int i_0xfb()        { m_wasEiInstruction = true; return 1; }
    inline int i_0xfc()        { ILLEGAL_INSTRUCTION(0xfc); return 0; }
    inline int i_0xfd()        { ILLEGAL_INSTRUCTION(0xfd); return 0; }
    inline int i_0xfe(u8 x)    { return cpARegAndValue(x); }
    inline int i_0xff()        { return callVector(JUMP_VECTOR_38); }

    // -----------------------  PREFIXED OPCODES -------------------------------

    int i_pref_0x00() { return rotateRegisterBitsLeftF(r8::B); }
    int i_pref_0x01() { return rotateRegisterBitsLeftF(r8::C); }
    int i_pref_0x02() { return rotateRegisterBitsLeftF(r8::D); }
    int i_pref_0x03() { return rotateRegisterBitsLeftF(r8::E); }
    int i_pref_0x04() { return rotateRegisterBitsLeftF(r8::H); }
    int i_pref_0x05() { return rotateRegisterBitsLeftF(r8::L); }
    int i_pref_0x06() { UNIMPLEMENTED(); return -1; }
    int i_pref_0x07() { return rotateRegisterBitsLeftF(r8::A); }
    int i_pref_0x08() { return rotateRegisterBitsRightF(r8::B); }
    int i_pref_0x09() { return rotateRegisterBitsRightF(r8::C); }
    int i_pref_0x0a() { return rotateRegisterBitsRightF(r8::D); }
    int i_pref_0x0b() { return rotateRegisterBitsRightF(r8::E); }
    int i_pref_0x0c() { return rotateRegisterBitsRightF(r8::H); }
    int i_pref_0x0d() { return rotateRegisterBitsRightF(r8::L); }
    int i_pref_0x0e() { UNIMPLEMENTED(); return -1; }
    int i_pref_0x0f() { return rotateRegisterBitsRightF(r8::L); }
    int i_pref_0x10() { return rotateRegisterBitsLeftThroughCarryF(r8::B); }
    int i_pref_0x11() { return rotateRegisterBitsLeftThroughCarryF(r8::C); }
    int i_pref_0x12() { return rotateRegisterBitsLeftThroughCarryF(r8::D); }
    int i_pref_0x13() { return rotateRegisterBitsLeftThroughCarryF(r8::E); }
    int i_pref_0x14() { return rotateRegisterBitsLeftThroughCarryF(r8::H); }
    int i_pref_0x15() { return rotateRegisterBitsLeftThroughCarryF(r8::L); }
    int i_pref_0x16() { UNIMPLEMENTED(); return -1; }
    int i_pref_0x17() { return rotateRegisterBitsLeftThroughCarryF(r8::A); }
    int i_pref_0x18() { return rotateRegisterBitsRightThroughCarryF(r8::B); }
    int i_pref_0x19() { return rotateRegisterBitsRightThroughCarryF(r8::C); }
    int i_pref_0x1a() { return rotateRegisterBitsRightThroughCarryF(r8::D); }
    int i_pref_0x1b() { return rotateRegisterBitsRightThroughCarryF(r8::E); }
    int i_pref_0x1c() { return rotateRegisterBitsRightThroughCarryF(r8::H); }
    int i_pref_0x1d() { return rotateRegisterBitsRightThroughCarryF(r8::L); }
    int i_pref_0x1e() { UNIMPLEMENTED(); return -1; }
    int i_pref_0x1f() { return rotateRegisterBitsRightThroughCarryF(r8::A); }
    int i_pref_0x20() { return shiftRegisterBitsLeftToCarryF(r8::B); }
    int i_pref_0x21() { return shiftRegisterBitsLeftToCarryF(r8::C); }
    int i_pref_0x22() { return shiftRegisterBitsLeftToCarryF(r8::D); }
    int i_pref_0x23() { return shiftRegisterBitsLeftToCarryF(r8::E); }
    int i_pref_0x24() { return shiftRegisterBitsLeftToCarryF(r8::H); }
    int i_pref_0x25() { return shiftRegisterBitsLeftToCarryF(r8::L); }
    int i_pref_0x26() { UNIMPLEMENTED(); return -1; }
    int i_pref_0x27() { return shiftRegisterBitsLeftToCarryF(r8::A); }
    int i_pref_0x28() { return shiftRegisterBitsRightToCarryF(r8::B); }
    int i_pref_0x29() { return shiftRegisterBitsRightToCarryF(r8::C); }
    int i_pref_0x2a() { return shiftRegisterBitsRightToCarryF(r8::D); }
    int i_pref_0x2b() { return shiftRegisterBitsRightToCarryF(r8::E); }
    int i_pref_0x2c() { return shiftRegisterBitsRightToCarryF(r8::H); }
    int i_pref_0x2d() { return shiftRegisterBitsRightToCarryF(r8::L); }
    int i_pref_0x2e() { UNIMPLEMENTED(); return -1; }
    int i_pref_0x2f() { return shiftRegisterBitsRightToCarryF(r8::A); }
    int i_pref_0x30() { return swapRegisterNibblesF(r8::B); }
    int i_pref_0x31() { return swapRegisterNibblesF(r8::C); }
    int i_pref_0x32() { return swapRegisterNibblesF(r8::D); }
    int i_pref_0x33() { return swapRegisterNibblesF(r8::E); }
    int i_pref_0x34() { return swapRegisterNibblesF(r8::H); }
    int i_pref_0x35() { return swapRegisterNibblesF(r8::L); }
    int i_pref_0x36() { UNIMPLEMENTED(); return -1; }
    int i_pref_0x37() { return swapRegisterNibblesF(r8::A); }
    int i_pref_0x38() { return shiftRightLogicRegisterF(r8::B); }
    int i_pref_0x39() { return shiftRightLogicRegisterF(r8::C); }
    int i_pref_0x3a() { return shiftRightLogicRegisterF(r8::D); }
    int i_pref_0x3b() { return shiftRightLogicRegisterF(r8::E); }
    int i_pref_0x3c() { return shiftRightLogicRegisterF(r8::H); }
    int i_pref_0x3d() { return shiftRightLogicRegisterF(r8::L); }
    int i_pref_0x3e() { UNIMPLEMENTED(); return -1; }
    int i_pref_0x3f() { return shiftRightLogicRegisterF(r8::A); }
    int i_pref_0x40() { return checkBitOfRegisterF(0, r8::B); }
    int i_pref_0x41() { return checkBitOfRegisterF(0, r8::C); }
    int i_pref_0x42() { return checkBitOfRegisterF(0, r8::D); }
    int i_pref_0x43() { return checkBitOfRegisterF(0, r8::E); }
    int i_pref_0x44() { return checkBitOfRegisterF(0, r8::H); }
    int i_pref_0x45() { return checkBitOfRegisterF(0, r8::L); }
    int i_pref_0x46() { UNIMPLEMENTED(); return -1; }
    int i_pref_0x47() { return checkBitOfRegisterF(0, r8::A); }
    int i_pref_0x48() { return checkBitOfRegisterF(1, r8::B); }
    int i_pref_0x49() { return checkBitOfRegisterF(1, r8::C); }
    int i_pref_0x4a() { return checkBitOfRegisterF(1, r8::D); }
    int i_pref_0x4b() { return checkBitOfRegisterF(1, r8::E); }
    int i_pref_0x4c() { return checkBitOfRegisterF(1, r8::H); }
    int i_pref_0x4d() { return checkBitOfRegisterF(1, r8::L); }
    int i_pref_0x4e() { UNIMPLEMENTED(); return -1; }
    int i_pref_0x4f() { return checkBitOfRegisterF(1, r8::A); }
    int i_pref_0x50() { return checkBitOfRegisterF(2, r8::B); }
    int i_pref_0x51() { return checkBitOfRegisterF(2, r8::C); }
    int i_pref_0x52() { return checkBitOfRegisterF(2, r8::D); }
    int i_pref_0x53() { return checkBitOfRegisterF(2, r8::E); }
    int i_pref_0x54() { return checkBitOfRegisterF(2, r8::H); }
    int i_pref_0x55() { return checkBitOfRegisterF(2, r8::L); }
    int i_pref_0x56() { UNIMPLEMENTED(); return -1; }
    int i_pref_0x57() { return checkBitOfRegisterF(2, r8::A); }
    int i_pref_0x58() { return checkBitOfRegisterF(3, r8::B); }
    int i_pref_0x59() { return checkBitOfRegisterF(3, r8::C); }
    int i_pref_0x5a() { return checkBitOfRegisterF(3, r8::D); }
    int i_pref_0x5b() { return checkBitOfRegisterF(3, r8::E); }
    int i_pref_0x5c() { return checkBitOfRegisterF(3, r8::H); }
    int i_pref_0x5d() { return checkBitOfRegisterF(3, r8::L); }
    int i_pref_0x5e() { UNIMPLEMENTED(); return -1; }
    int i_pref_0x5f() { return checkBitOfRegisterF(3, r8::A); }
    int i_pref_0x60() { return checkBitOfRegisterF(4, r8::B); }
    int i_pref_0x61() { return checkBitOfRegisterF(4, r8::C); }
    int i_pref_0x62() { return checkBitOfRegisterF(4, r8::D); }
    int i_pref_0x63() { return checkBitOfRegisterF(4, r8::E); }
    int i_pref_0x64() { return checkBitOfRegisterF(4, r8::H); }
    int i_pref_0x65() { return checkBitOfRegisterF(4, r8::L); }
    int i_pref_0x66() { UNIMPLEMENTED(); return -1; }
    int i_pref_0x67() { return checkBitOfRegisterF(4, r8::A); }
    int i_pref_0x68() { return checkBitOfRegisterF(5, r8::B); }
    int i_pref_0x69() { return checkBitOfRegisterF(5, r8::C); }
    int i_pref_0x6a() { return checkBitOfRegisterF(5, r8::D); }
    int i_pref_0x6b() { return checkBitOfRegisterF(5, r8::E); }
    int i_pref_0x6c() { return checkBitOfRegisterF(5, r8::H); }
    int i_pref_0x6d() { return checkBitOfRegisterF(5, r8::L); }
    int i_pref_0x6e() { UNIMPLEMENTED(); return -1; }
    int i_pref_0x6f() { return checkBitOfRegisterF(5, r8::A); }
    int i_pref_0x70() { return checkBitOfRegisterF(6, r8::B); }
    int i_pref_0x71() { return checkBitOfRegisterF(6, r8::C); }
    int i_pref_0x72() { return checkBitOfRegisterF(6, r8::D); }
    int i_pref_0x73() { return checkBitOfRegisterF(6, r8::E); }
    int i_pref_0x74() { return checkBitOfRegisterF(6, r8::H); }
    int i_pref_0x75() { return checkBitOfRegisterF(6, r8::L); }
    int i_pref_0x76() { UNIMPLEMENTED(); return -1; }
    int i_pref_0x77() { return checkBitOfRegisterF(6, r8::A); }
    int i_pref_0x78() { return checkBitOfRegisterF(7, r8::B); }
    int i_pref_0x79() { return checkBitOfRegisterF(7, r8::C); }
    int i_pref_0x7a() { return checkBitOfRegisterF(7, r8::D); }
    int i_pref_0x7b() { return checkBitOfRegisterF(7, r8::E); }
    int i_pref_0x7c() { return checkBitOfRegisterF(7, r8::H); }
    int i_pref_0x7d() { return checkBitOfRegisterF(7, r8::L); }
    int i_pref_0x7e() { UNIMPLEMENTED(); return -1; }
    int i_pref_0x7f() { return checkBitOfRegisterF(7, r8::A); }
    int i_pref_0x80() { return resetBitOfRegister(0, r8::B); }
    int i_pref_0x81() { return resetBitOfRegister(0, r8::C); }
    int i_pref_0x82() { return resetBitOfRegister(0, r8::D); }
    int i_pref_0x83() { return resetBitOfRegister(0, r8::E); }
    int i_pref_0x84() { return resetBitOfRegister(0, r8::H); }
    int i_pref_0x85() { return resetBitOfRegister(0, r8::L); }
    int i_pref_0x86() { UNIMPLEMENTED(); return -1; }
    int i_pref_0x87() { return resetBitOfRegister(0, r8::A); }
    int i_pref_0x88() { return resetBitOfRegister(1, r8::B); }
    int i_pref_0x89() { return resetBitOfRegister(1, r8::C); }
    int i_pref_0x8a() { return resetBitOfRegister(1, r8::D); }
    int i_pref_0x8b() { return resetBitOfRegister(1, r8::E); }
    int i_pref_0x8c() { return resetBitOfRegister(1, r8::H); }
    int i_pref_0x8d() { return resetBitOfRegister(1, r8::L); }
    int i_pref_0x8e() { UNIMPLEMENTED(); return -1; }
    int i_pref_0x8f() { return resetBitOfRegister(1, r8::A); }
    int i_pref_0x90() { return resetBitOfRegister(2, r8::B); }
    int i_pref_0x91() { return resetBitOfRegister(2, r8::C); }
    int i_pref_0x92() { return resetBitOfRegister(2, r8::D); }
    int i_pref_0x93() { return resetBitOfRegister(2, r8::E); }
    int i_pref_0x94() { return resetBitOfRegister(2, r8::H); }
    int i_pref_0x95() { return resetBitOfRegister(2, r8::L); }
    int i_pref_0x96() { UNIMPLEMENTED(); return -1; }
    int i_pref_0x97() { return resetBitOfRegister(2, r8::A); }
    int i_pref_0x98() { return resetBitOfRegister(3, r8::B); }
    int i_pref_0x99() { return resetBitOfRegister(3, r8::C); }
    int i_pref_0x9a() { return resetBitOfRegister(3, r8::D); }
    int i_pref_0x9b() { return resetBitOfRegister(3, r8::E); }
    int i_pref_0x9c() { return resetBitOfRegister(3, r8::H); }
    int i_pref_0x9d() { return resetBitOfRegister(3, r8::L); }
    int i_pref_0x9e() { UNIMPLEMENTED(); return -1; }
    int i_pref_0x9f() { return resetBitOfRegister(3, r8::A); }
    int i_pref_0xa0() { return resetBitOfRegister(4, r8::B); }
    int i_pref_0xa1() { return resetBitOfRegister(4, r8::C); }
    int i_pref_0xa2() { return resetBitOfRegister(4, r8::D); }
    int i_pref_0xa3() { return resetBitOfRegister(4, r8::E); }
    int i_pref_0xa4() { return resetBitOfRegister(4, r8::H); }
    int i_pref_0xa5() { return resetBitOfRegister(4, r8::L); }
    int i_pref_0xa6() { UNIMPLEMENTED(); return -1; }
    int i_pref_0xa7() { return resetBitOfRegister(4, r8::A); }
    int i_pref_0xa8() { return resetBitOfRegister(5, r8::B); }
    int i_pref_0xa9() { return resetBitOfRegister(5, r8::C); }
    int i_pref_0xaa() { return resetBitOfRegister(5, r8::D); }
    int i_pref_0xab() { return resetBitOfRegister(5, r8::E); }
    int i_pref_0xac() { return resetBitOfRegister(5, r8::H); }
    int i_pref_0xad() { return resetBitOfRegister(5, r8::L); }
    int i_pref_0xae() { UNIMPLEMENTED(); return -1; }
    int i_pref_0xaf() { return resetBitOfRegister(5, r8::A); }
    int i_pref_0xb0() { return resetBitOfRegister(6, r8::B); }
    int i_pref_0xb1() { return resetBitOfRegister(6, r8::C); }
    int i_pref_0xb2() { return resetBitOfRegister(6, r8::D); }
    int i_pref_0xb3() { return resetBitOfRegister(6, r8::E); }
    int i_pref_0xb4() { return resetBitOfRegister(6, r8::H); }
    int i_pref_0xb5() { return resetBitOfRegister(6, r8::L); }
    int i_pref_0xb6() { UNIMPLEMENTED(); return -1; }
    int i_pref_0xb7() { return resetBitOfRegister(6, r8::A); }
    int i_pref_0xb8() { return resetBitOfRegister(7, r8::B); }
    int i_pref_0xb9() { return resetBitOfRegister(7, r8::C); }
    int i_pref_0xba() { return resetBitOfRegister(7, r8::D); }
    int i_pref_0xbb() { return resetBitOfRegister(7, r8::E); }
    int i_pref_0xbc() { return resetBitOfRegister(7, r8::H); }
    int i_pref_0xbd() { return resetBitOfRegister(7, r8::L); }
    int i_pref_0xbe() { UNIMPLEMENTED(); return -1; }
    int i_pref_0xbf() { return resetBitOfRegister(7, r8::A); }
    int i_pref_0xc0() { return setBitOfRegister(0, r8::B); }
    int i_pref_0xc1() { return setBitOfRegister(0, r8::C); }
    int i_pref_0xc2() { return setBitOfRegister(0, r8::D); }
    int i_pref_0xc3() { return setBitOfRegister(0, r8::E); }
    int i_pref_0xc4() { return setBitOfRegister(0, r8::H); }
    int i_pref_0xc5() { return setBitOfRegister(0, r8::L); }
    int i_pref_0xc6() { UNIMPLEMENTED(); return -1; }
    int i_pref_0xc7() { return setBitOfRegister(0, r8::A); }
    int i_pref_0xc8() { return setBitOfRegister(1, r8::B); }
    int i_pref_0xc9() { return setBitOfRegister(1, r8::C); }
    int i_pref_0xca() { return setBitOfRegister(1, r8::D); }
    int i_pref_0xcb() { return setBitOfRegister(1, r8::E); }
    int i_pref_0xcc() { return setBitOfRegister(1, r8::H); }
    int i_pref_0xcd() { return setBitOfRegister(1, r8::L); }
    int i_pref_0xce() { UNIMPLEMENTED(); return -1; }
    int i_pref_0xcf() { return setBitOfRegister(1, r8::A); }
    int i_pref_0xd0() { return setBitOfRegister(2, r8::B); }
    int i_pref_0xd1() { return setBitOfRegister(2, r8::C); }
    int i_pref_0xd2() { return setBitOfRegister(2, r8::D); }
    int i_pref_0xd3() { return setBitOfRegister(2, r8::E); }
    int i_pref_0xd4() { return setBitOfRegister(2, r8::H); }
    int i_pref_0xd5() { return setBitOfRegister(2, r8::L); }
    int i_pref_0xd6() { UNIMPLEMENTED(); return -1; }
    int i_pref_0xd7() { return setBitOfRegister(2, r8::A); }
    int i_pref_0xd8() { return setBitOfRegister(3, r8::B); }
    int i_pref_0xd9() { return setBitOfRegister(3, r8::C); }
    int i_pref_0xda() { return setBitOfRegister(3, r8::D); }
    int i_pref_0xdb() { return setBitOfRegister(3, r8::E); }
    int i_pref_0xdc() { return setBitOfRegister(3, r8::H); }
    int i_pref_0xdd() { return setBitOfRegister(3, r8::L); }
    int i_pref_0xde() { UNIMPLEMENTED(); return -1; }
    int i_pref_0xdf() { return setBitOfRegister(3, r8::A); }
    int i_pref_0xe0() { return setBitOfRegister(4, r8::B); }
    int i_pref_0xe1() { return setBitOfRegister(4, r8::C); }
    int i_pref_0xe2() { return setBitOfRegister(4, r8::D); }
    int i_pref_0xe3() { return setBitOfRegister(4, r8::E); }
    int i_pref_0xe4() { return setBitOfRegister(4, r8::H); }
    int i_pref_0xe5() { return setBitOfRegister(4, r8::L); }
    int i_pref_0xe6() { UNIMPLEMENTED(); return -1; }
    int i_pref_0xe7() { return setBitOfRegister(4, r8::A); }
    int i_pref_0xe8() { return setBitOfRegister(5, r8::B); }
    int i_pref_0xe9() { return setBitOfRegister(5, r8::C); }
    int i_pref_0xea() { return setBitOfRegister(5, r8::D); }
    int i_pref_0xeb() { return setBitOfRegister(5, r8::E); }
    int i_pref_0xec() { return setBitOfRegister(5, r8::H); }
    int i_pref_0xed() { return setBitOfRegister(5, r8::L); }
    int i_pref_0xee() { UNIMPLEMENTED(); return -1; }
    int i_pref_0xef() { return setBitOfRegister(5, r8::A); }
    int i_pref_0xf0() { return setBitOfRegister(6, r8::B); }
    int i_pref_0xf1() { return setBitOfRegister(6, r8::C); }
    int i_pref_0xf2() { return setBitOfRegister(6, r8::D); }
    int i_pref_0xf3() { return setBitOfRegister(6, r8::E); }
    int i_pref_0xf4() { return setBitOfRegister(6, r8::H); }
    int i_pref_0xf5() { return setBitOfRegister(6, r8::L); }
    int i_pref_0xf6() { UNIMPLEMENTED(); return -1; }
    int i_pref_0xf7() { return setBitOfRegister(6, r8::A); }
    int i_pref_0xf8() { return setBitOfRegister(7, r8::B); }
    int i_pref_0xf9() { return setBitOfRegister(7, r8::C); }
    int i_pref_0xfa() { return setBitOfRegister(7, r8::D); }
    int i_pref_0xfb() { return setBitOfRegister(7, r8::E); }
    int i_pref_0xfc() { return setBitOfRegister(7, r8::H); }
    int i_pref_0xfd() { return setBitOfRegister(7, r8::L); }
    int i_pref_0xfe() { UNIMPLEMENTED(); return -1; }
    int i_pref_0xff() { return setBitOfRegister(7, r8::A); }
};



#endif /* CPU_H_ */
