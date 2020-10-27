#ifndef REGISTERS_H_
#define REGISTERS_H_

#include "config.h"

#include "common.h"
#include "Logger.h"
#include "string_formatting.h"

#include <stdint.h>

class Registers final
{
private:
    uint8_t     m_A{};
    uint8_t     m_B{};
    uint8_t     m_C{};
    uint8_t     m_D{};
    uint8_t     m_E{};
    uint8_t     m_F{};
    uint8_t     m_H{};
    uint8_t     m_L{};

    uint16_t    m_SP{};
    uint16_t    m_PC{};

    // Interrupt master enable - write only
    bool        m_ime{};

public:
    enum class r8{A, B, C, D, E, F, H, L};
    enum class r16{AF, BC, DE, HL, SP, PC};
    enum class flag{Z, NZ, C, NC};

    Registers();

    // --- 8-bit registers ---

    // -- get --
    inline uint8_t getA() const         { return m_A; Logger::info("Value of register A got"); }
    inline uint8_t getB() const         { return m_B; Logger::info("Value of register B got"); }
    inline uint8_t getC() const         { return m_C; Logger::info("Value of register C got"); }
    inline uint8_t getD() const         { return m_D; Logger::info("Value of register D got"); }
    inline uint8_t getE() const         { return m_E; Logger::info("Value of register E got"); }
    inline uint8_t getF() const         { return m_F; Logger::info("Value of register F got"); }
    inline uint8_t getH() const         { return m_H; Logger::info("Value of register H got"); }
    inline uint8_t getL() const         { return m_L; Logger::info("Value of register L got"); }

    inline uint8_t get8(r8 reg) const
    {
        switch (reg)
        {
        case r8::A: return getA();
        case r8::B: return getB();
        case r8::C: return getC();
        case r8::D: return getD();
        case r8::E: return getE();
        case r8::F: return getF();
        case r8::H: return getH();
        case r8::L: return getL();
        default: IMPOSSIBLE();
        }
    }

    // -- set --
    inline void setA(uint8_t value)     { m_A = value; Logger::info("Value of register A set to: " + toHexStr(value)); }
    inline void setB(uint8_t value)     { m_B = value; Logger::info("Value of register B set to: " + toHexStr(value)); }
    inline void setC(uint8_t value)     { m_C = value; Logger::info("Value of register C set to: " + toHexStr(value)); }
    inline void setD(uint8_t value)     { m_D = value; Logger::info("Value of register D set to: " + toHexStr(value)); }
    inline void setE(uint8_t value)     { m_E = value; Logger::info("Value of register E set to: " + toHexStr(value)); }
    inline void setF(uint8_t value)     { m_F = value; resetFlagRegisterLowerBits(); Logger::info("Value of register F set to: " + toHexStr(value)); }
    inline void setH(uint8_t value)     { m_H = value; Logger::info("Value of register H set to: " + toHexStr(value)); }
    inline void setL(uint8_t value)     { m_L = value; Logger::info("Value of register L set to: " + toHexStr(value)); }

    inline void set8(r8 reg, uint8_t value)
    {
        switch (reg)
        {
        case r8::A: return setA(value); break;
        case r8::B: return setB(value); break;
        case r8::C: return setC(value); break;
        case r8::D: return setD(value); break;
        case r8::E: return setE(value); break;
        case r8::F: return setF(value); break;
        case r8::H: return setH(value); break;
        case r8::L: return setL(value); break;
        default: IMPOSSIBLE();
        }
    }


    // --- 16-bit registers ---

    // -- get --
    inline uint16_t getAF() const       { return (uint16_t)m_A << 8 |
                                                 (uint16_t)m_F;         }
    inline uint16_t getBC() const       { return (uint16_t)m_B << 8 |
                                                 (uint16_t)m_C;         }
    inline uint16_t getDE() const       { return (uint16_t)m_D << 8 |
                                                 (uint16_t)m_E;         }
    inline uint16_t getHL() const       { return (uint16_t)m_H << 8 |
                                                 (uint16_t)m_L;         }
    inline uint16_t getSP() const       { return m_SP;                  }
    inline uint16_t getPC() const       { return m_PC;                  }

    inline uint16_t get16(r16 reg) const
    {
        switch (reg)
        {
        case r16::AF: return getAF();
        case r16::BC: return getBC();
        case r16::DE: return getDE();
        case r16::HL: return getHL();
        case r16::SP: return getSP();
        case r16::PC: return getPC();
        default: IMPOSSIBLE();
        }
    }

    // -- set --
    inline void setAF(uint16_t value)   { m_A = (value & 0xff00) >> 8;
                                          m_F =  value & 0x00ff; resetFlagRegisterLowerBits(); }
    inline void setBC(uint16_t value)   { m_B = (value & 0xff00) >> 8;
                                          m_C =  value & 0x00ff;        }
    inline void setDE(uint16_t value)   { m_D = (value & 0xff00) >> 8;
                                          m_E =  value & 0x00ff;        }
    inline void setHL(uint16_t value)   { m_H = (value & 0xff00) >> 8;
                                          m_L =  value & 0x00ff;        }
    inline void setSP(uint16_t value)   { m_SP = value;                 }
    inline void setPC(uint16_t value)   { m_PC = value;                 }

    inline void set16(r16 reg, uint16_t value)
    {
        switch (reg)
        {
        case r16::AF: setAF(value); break;
        case r16::BC: setBC(value); break;
        case r16::DE: setDE(value); break;
        case r16::HL: setHL(value); break;
        case r16::SP: setSP(value); break;
        case r16::PC: setPC(value); break;
        }
    }

    // -- increment --
    inline void incrementSP(uint16_t val=1) { m_SP += val; }

    // -- decrement --
    inline void decrementSP(uint16_t val=1) { m_SP -= val; }


    // --- flag register (F) ---

    // bit 7   -   zero flag                    -   zf
    // bit 6   -   add/sub-flag/negative flag   -   n
    // bit 5   -   half carry flag              -   h
    // bit 4   -   carry flag                   -   cy

    // The lower 4 bits of the flag register must always be 0 even after a write.
    inline void resetFlagRegisterLowerBits() { m_F &= 0xf0; }

    // -- get --
    inline uint8_t getZeroFlag()        const { return (m_F & 0b10000000) >>  7; }
    inline uint8_t getNegativeFlag()    const { return (m_F & 0b01000000) >>  6; }
    inline uint8_t getHalfCarryFlag()   const { return (m_F & 0b00100000) >>  5; }
    inline uint8_t getCarryFlag()       const { return (m_F & 0b00010000) >>  4; }

    inline uint8_t getFlag(flag f)
    {
        switch (f)
        {
            case flag::Z: return getZeroFlag();;
            case flag::NZ: return !getZeroFlag();
            case flag::C: return getCarryFlag();
            case flag::NC: return !getCarryFlag();
            default: IMPOSSIBLE();
        }
    }

    // -- set --
    inline void setZeroFlag()       { m_F |=  0b10000000; }
    inline void setNegativeFlag()   { m_F |=  0b01000000; }
    inline void setHalfCarryFlag()  { m_F |=  0b00100000; }
    inline void setCarryFlag()      { m_F |=  0b00010000; }

    // unset
    inline void unsetZeroFlag()         { m_F &= ~0b10000000; }
    inline void unsetNegativeFlag()     { m_F &= ~0b01000000; }
    inline void unsetHalfCarryFlag()    { m_F &= ~0b00100000; }
    inline void unsetCarryFlag()        { m_F &= ~0b00010000; }

    // set depending on the argument
    inline void setZeroFlag(uint8_t value)       { value ? setZeroFlag()        : unsetZeroFlag();          }
    inline void setNegativeFlag(uint8_t value)   { value ? setNegativeFlag()    : unsetNegativeFlag();      }
    inline void setHalfCarryFlag(uint8_t value)  { value ? setHalfCarryFlag()   : unsetHalfCarryFlag();     }
    inline void setCarryFlag(uint8_t value)      { value ? setCarryFlag()       : unsetCarryFlag();         }


    // --- misc. registers ---


    // -- get --
    inline bool getIme() const { return m_ime; } // Only for internal use

    // -- set --
    inline void setIme() { m_ime = true; }

    // -- unset --
    inline void unsetIme() { m_ime = false; }
};


#endif /* REGISTERS_H_ */
