#ifndef OPCODE_NAMES_H_
#define OPCODE_NAMES_H_

#include <string_view>
#include <string>
#include <array>
#include <stdint.h>

class OpcodeNames final
{
private:
    static constexpr std::array<std::string_view, 256> m_opcodeNames{
        "NOP",            // 0x0
        "LD BC,u16",      // 0x1
        "LD (BC),A",      // 0x2
        "INC BC",         // 0x3
        "INC B",          // 0x4
        "DEC B",          // 0x5
        "LD B,u8",        // 0x6
        "RLCA",           // 0x7
        "LD (u16),SP",    // 0x8
        "ADD HL,BC",      // 0x9
        "LD A,(BC)",      // 0xa
        "DEC BC",         // 0xb
        "INC C",          // 0xc
        "DEC C",          // 0xd
        "LD C,u8",        // 0xe
        "RRCA",           // 0xf
        "STOP",           // 0x10
        "LD DE,u16",      // 0x11
        "LD (DE),A",      // 0x12
        "INC DE",         // 0x13
        "INC D",          // 0x14
        "DEC D",          // 0x15
        "LD D,u8",        // 0x16
        "RLA",            // 0x17
        "JR i8",          // 0x18
        "ADD HL,DE",      // 0x19
        "LD A,(DE)",      // 0x1a
        "DEC DE",         // 0x1b
        "INC E",          // 0x1c
        "DEC E",          // 0x1d
        "LD E,u8",        // 0x1e
        "RRA",            // 0x1f
        "JR NZ,i8",       // 0x20
        "LD HL,u16",      // 0x21
        "LD (HL+),A",     // 0x22
        "INC HL",         // 0x23
        "INC H",          // 0x24
        "DEC H",          // 0x25
        "LD H,u8",        // 0x26
        "DAA",            // 0x27
        "JR Z,i8",        // 0x28
        "ADD HL,HL",      // 0x29
        "LD A,(HL+)",     // 0x2a
        "DEC HL",         // 0x2b
        "INC L",          // 0x2c
        "DEC L",          // 0x2d
        "LD L,u8",        // 0x2e
        "CPL",            // 0x2f
        "JR NC,i8",       // 0x30
        "LD SP,u16",      // 0x31
        "LD (HL-),A",     // 0x32
        "INC SP",         // 0x33
        "INC (HL)",       // 0x34
        "DEC (HL)",       // 0x35
        "LD (HL),u8",     // 0x36
        "SCF",            // 0x37
        "JR C,i8",        // 0x38
        "ADD HL,SP",      // 0x39
        "LD A,(HL-)",     // 0x3a
        "DEC SP",         // 0x3b
        "INC A",          // 0x3c
        "DEC A",          // 0x3d
        "LD A,u8",        // 0x3e
        "CCF",            // 0x3f
        "LD B,B",         // 0x40
        "LD B,C",         // 0x41
        "LD B,D",         // 0x42
        "LD B,E",         // 0x43
        "LD B,H",         // 0x44
        "LD B,L",         // 0x45
        "LD B,(HL)",      // 0x46
        "LD B,A",         // 0x47
        "LD C,B",         // 0x48
        "LD C,C",         // 0x49
        "LD C,D",         // 0x4a
        "LD C,E",         // 0x4b
        "LD C,H",         // 0x4c
        "LD C,L",         // 0x4d
        "LD C,(HL)",      // 0x4e
        "LD C,A",         // 0x4f
        "LD D,B",         // 0x50
        "LD D,C",         // 0x51
        "LD D,D",         // 0x52
        "LD D,E",         // 0x53
        "LD D,H",         // 0x54
        "LD D,L",         // 0x55
        "LD D,(HL)",      // 0x56
        "LD D,A",         // 0x57
        "LD E,B",         // 0x58
        "LD E,C",         // 0x59
        "LD E,D",         // 0x5a
        "LD E,E",         // 0x5b
        "LD E,H",         // 0x5c
        "LD E,L",         // 0x5d
        "LD E,(HL)",      // 0x5e
        "LD E,A",         // 0x5f
        "LD H,B",         // 0x60
        "LD H,C",         // 0x61
        "LD H,D",         // 0x62
        "LD H,E",         // 0x63
        "LD H,H",         // 0x64
        "LD H,L",         // 0x65
        "LD H,(HL)",      // 0x66
        "LD H,A",         // 0x67
        "LD L,B",         // 0x68
        "LD L,C",         // 0x69
        "LD L,D",         // 0x6a
        "LD L,E",         // 0x6b
        "LD L,H",         // 0x6c
        "LD L,L",         // 0x6d
        "LD L,(HL)",      // 0x6e
        "LD L,A",         // 0x6f
        "LD (HL),B",      // 0x70
        "LD (HL),C",      // 0x71
        "LD (HL),D",      // 0x72
        "LD (HL),E",      // 0x73
        "LD (HL),H",      // 0x74
        "LD (HL),L",      // 0x75
        "HALT",           // 0x76
        "LD (HL),A",      // 0x77
        "LD A,B",         // 0x78
        "LD A,C",         // 0x79
        "LD A,D",         // 0x7a
        "LD A,E",         // 0x7b
        "LD A,H",         // 0x7c
        "LD A,L",         // 0x7d
        "LD A,(HL)",      // 0x7e
        "LD A,A",         // 0x7f
        "ADD A,B",        // 0x80
        "ADD A,C",        // 0x81
        "ADD A,D",        // 0x82
        "ADD A,E",        // 0x83
        "ADD A,H",        // 0x84
        "ADD A,L",        // 0x85
        "ADD A,(HL)",     // 0x86
        "ADD A,A",        // 0x87
        "ADC A,B",        // 0x88
        "ADC A,C",        // 0x89
        "ADC A,D",        // 0x8a
        "ADC A,E",        // 0x8b
        "ADC A,H",        // 0x8c
        "ADC A,L",        // 0x8d
        "ADC A,(HL)",     // 0x8e
        "ADC A,A",        // 0x8f
        "SUB A,B",        // 0x90
        "SUB A,C",        // 0x91
        "SUB A,D",        // 0x92
        "SUB A,E",        // 0x93
        "SUB A,H",        // 0x94
        "SUB A,L",        // 0x95
        "SUB A,(HL)",     // 0x96
        "SUB A,A",        // 0x97
        "SBC A,B",        // 0x98
        "SBC A,C",        // 0x99
        "SBC A,D",        // 0x9a
        "SBC A,E",        // 0x9b
        "SBC A,H",        // 0x9c
        "SBC A,L",        // 0x9d
        "SBC A,(HL)",     // 0x9e
        "SBC A,A",        // 0x9f
        "AND A,B",        // 0xa0
        "AND A,C",        // 0xa1
        "AND A,D",        // 0xa2
        "AND A,E",        // 0xa3
        "AND A,H",        // 0xa4
        "AND A,L",        // 0xa5
        "AND A,(HL)",     // 0xa6
        "AND A,A",        // 0xa7
        "XOR A,B",        // 0xa8
        "XOR A,C",        // 0xa9
        "XOR A,D",        // 0xaa
        "XOR A,E",        // 0xab
        "XOR A,H",        // 0xac
        "XOR A,L",        // 0xad
        "XOR A,(HL)",     // 0xae
        "XOR A,A",        // 0xaf
        "OR A,B",         // 0xb0
        "OR A,C",         // 0xb1
        "OR A,D",         // 0xb2
        "OR A,E",         // 0xb3
        "OR A,H",         // 0xb4
        "OR A,L",         // 0xb5
        "OR A,(HL)",      // 0xb6
        "OR A,A",         // 0xb7
        "CP A,B",         // 0xb8
        "CP A,C",         // 0xb9
        "CP A,D",         // 0xba
        "CP A,E",         // 0xbb
        "CP A,H",         // 0xbc
        "CP A,L",         // 0xbd
        "CP A,(HL)",      // 0xbe
        "CP A,A",         // 0xbf
        "RET NZ",         // 0xc0
        "POP BC",         // 0xc1
        "JP NZ,u16",      // 0xc2
        "JP u16",         // 0xc3
        "CALL NZ,u16",    // 0xc4
        "PUSH BC",        // 0xc5
        "ADD A,u8",       // 0xc6
        "RST 00h",        // 0xc7
        "RET Z",          // 0xc8
        "RET",            // 0xc9
        "JP Z,u16",       // 0xca
        "PREFIX CB",      // 0xcb
        "CALL Z,u16",     // 0xcc
        "CALL u16",       // 0xcd
        "ADC A,u8",       // 0xce
        "RST 08h",        // 0xcf
        "RET NC",         // 0xd0
        "POP DE",         // 0xd1
        "JP NC,u16",      // 0xd2
        "UNUSED",         // 0xd3
        "CALL NC,u16",    // 0xd4
        "PUSH DE",        // 0xd5
        "SUB A,u8",       // 0xd6
        "RST 10h",        // 0xd7
        "RET C",          // 0xd8
        "RETI",           // 0xd9
        "JP C,u16",       // 0xda
        "UNUSED",         // 0xdb
        "CALL C,u16",     // 0xdc
        "UNUSED",         // 0xdd
        "SBC A,u8",       // 0xde
        "RST 18h",        // 0xdf
        "LD (FF00+u8),A", // 0xe0
        "POP HL",         // 0xe1
        "LD (FF00+C),A",  // 0xe2
        "UNUSED",         // 0xe3
        "UNUSED",         // 0xe4
        "PUSH HL",        // 0xe5
        "AND A,u8",       // 0xe6
        "RST 20h",        // 0xe7
        "ADD SP,i8",      // 0xe8
        "JP HL",          // 0xe9
        "LD (u16),A",     // 0xea
        "UNUSED",         // 0xeb
        "UNUSED",         // 0xec
        "UNUSED",         // 0xed
        "XOR A,u8",       // 0xee
        "RST 28h",        // 0xef
        "LD A,(FF00+u8)", // 0xf0
        "POP AF",         // 0xf1
        "LD A,(FF00+C)",  // 0xf2
        "DI",             // 0xf3
        "UNUSED",         // 0xf4
        "PUSH AF",        // 0xf5
        "OR A,u8",        // 0xf6
        "RST 30h",        // 0xf7
        "LD HL,SP+i8",    // 0xf8
        "LD SP,HL",       // 0xf9
        "LD A,(u16)",     // 0xfa
        "EI",             // 0xfb
        "UNUSED",         // 0xfc
        "UNUSED",         // 0xfd
        "CP A,u8",        // 0xfe
        "RST 38h",        // 0xff
};

public:
    static inline const std::string get(uint8_t index) { return std::string(m_opcodeNames[index]); }
};

#endif /* OPCODE_NAMES_H_ */