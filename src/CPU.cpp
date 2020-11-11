#include "CPU.h"
#include "opcode_sizes.h"

#include "common.h"

CPU::CPU(Memory *memory)
{
    m_registers = new Registers;
    m_memoryPtr = memory;
}

void CPU::fetchOpcode()
{
    uint32_t bytesAtPc{m_memoryPtr->getOpcodeNoSwap(m_registers->getPC())};
    int opcodeSize{opcodeSizes[bytesAtPc >> 24]};

    switch (opcodeSize)
    {
    case 1:
        m_currentOpcode = bytesAtPc & 0xff000000;
        break;
    case 2:
        m_currentOpcode = bytesAtPc & 0xffff0000;
        break;
    case 3:
        m_currentOpcode =  (bytesAtPc & 0xff000000) |
                          ((bytesAtPc & 0x00ff0000) >> 8) |
                          ((bytesAtPc & 0x0000ff00) << 8);
        break;
    default:
        IMPOSSIBLE();
    }

    m_opcodeSize = opcodeSize;
}

void CPU::handleInterrupts()
{
    for (int i{}; i < 4; ++i)
    {
        // If interrrupts are disabled, exit
        if (!m_registers->getIme()) break;

        // Interrupt enable
        uint8_t ieValue{m_memoryPtr->get(REGISTER_ADDR_IE, false)};
        // Interrupt request
        uint8_t ifValue{m_memoryPtr->get(REGISTER_ADDR_IF, false)};

        // If the interrupt is enabled and is requested
        if (ieValue & (1 << i) && ifValue & (1 << i))
        {
            Logger::info("Handling interrupt: "+toHexStr(m_interruptHandlers[i]));

            m_registers->unsetIme();

            // Call the handler
            call(m_interruptHandlers[i]);

            // Reset the current bit
            ifValue &= ~(1 << i);

            // Feed back the new value of IF
            m_memoryPtr->set(REGISTER_ADDR_IF, ifValue, false);
        }
    }
}

void CPU::emulateCurrentOpcode()
{
    m_wasJump = false;

    switch (m_currentOpcode >> 24)
    {
    case 0x00: i_0x00();                                    break;
    case 0x01: i_0x01((m_currentOpcode&0x00ffff00)>>8);     break;
    case 0x02: i_0x02();                                    break;
    case 0x03: i_0x03();                                    break;
    case 0x04: i_0x04();                                    break;
    case 0x05: i_0x05();                                    break;
    case 0x06: i_0x06((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0x07: i_0x07();                                    break;
    case 0x08: i_0x08((m_currentOpcode&0x00ffff00)>>8);     break;
    case 0x09: i_0x09();                                    break;
    case 0x0a: i_0x0a();                                    break;
    case 0x0b: i_0x0b();                                    break;
    case 0x0c: i_0x0c();                                    break;
    case 0x0d: i_0x0d();                                    break;
    case 0x0e: i_0x0e((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0x0f: i_0x0f();                                    break;
    case 0x10: i_0x10();                                    break;
    case 0x11: i_0x11((m_currentOpcode&0x00ffff00)>>8);     break;
    case 0x12: i_0x12();                                    break;
    case 0x13: i_0x13();                                    break;
    case 0x14: i_0x14();                                    break;
    case 0x15: i_0x15();                                    break;
    case 0x16: i_0x16((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0x17: i_0x17();                                    break;
    case 0x18: i_0x18((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0x19: i_0x19();                                    break;
    case 0x1a: i_0x1a();                                    break;
    case 0x1b: i_0x1b();                                    break;
    case 0x1c: i_0x1c();                                    break;
    case 0x1d: i_0x1d();                                    break;
    case 0x1e: i_0x1e((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0x1f: i_0x1f();                                    break;
    case 0x20: i_0x20((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0x21: i_0x21((m_currentOpcode&0x00ffff00)>>8);     break;
    case 0x22: i_0x22();                                    break;
    case 0x23: i_0x23();                                    break;
    case 0x24: i_0x24();                                    break;
    case 0x25: i_0x25();                                    break;
    case 0x26: i_0x26((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0x27: i_0x27();                                    break;
    case 0x28: i_0x28((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0x29: i_0x29();                                    break;
    case 0x2a: i_0x2a();                                    break;
    case 0x2b: i_0x2b();                                    break;
    case 0x2c: i_0x2c();                                    break;
    case 0x2d: i_0x2d();                                    break;
    case 0x2e: i_0x2e((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0x2f: i_0x2f();                                    break;
    case 0x30: i_0x30((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0x31: i_0x31((m_currentOpcode&0x00ffff00)>>8);     break;
    case 0x32: i_0x32();                                    break;
    case 0x33: i_0x33();                                    break;
    case 0x34: i_0x34();                                    break;
    case 0x35: i_0x35();                                    break;
    case 0x36: i_0x36((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0x37: i_0x37();                                    break;
    case 0x38: i_0x38((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0x39: i_0x39();                                    break;
    case 0x3a: i_0x3a();                                    break;
    case 0x3b: i_0x3b();                                    break;
    case 0x3c: i_0x3c();                                    break;
    case 0x3d: i_0x3d();                                    break;
    case 0x3e: i_0x3e((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0x3f: i_0x3f();                                    break;
    case 0x40: i_0x40();                                    break;
    case 0x41: i_0x41();                                    break;
    case 0x42: i_0x42();                                    break;
    case 0x43: i_0x43();                                    break;
    case 0x44: i_0x44();                                    break;
    case 0x45: i_0x45();                                    break;
    case 0x46: i_0x46();                                    break;
    case 0x47: i_0x47();                                    break;
    case 0x48: i_0x48();                                    break;
    case 0x49: i_0x49();                                    break;
    case 0x4a: i_0x4a();                                    break;
    case 0x4b: i_0x4b();                                    break;
    case 0x4c: i_0x4c();                                    break;
    case 0x4d: i_0x4d();                                    break;
    case 0x4e: i_0x4e();                                    break;
    case 0x4f: i_0x4f();                                    break;
    case 0x50: i_0x50();                                    break;
    case 0x51: i_0x51();                                    break;
    case 0x52: i_0x52();                                    break;
    case 0x53: i_0x53();                                    break;
    case 0x54: i_0x54();                                    break;
    case 0x55: i_0x55();                                    break;
    case 0x56: i_0x56();                                    break;
    case 0x57: i_0x57();                                    break;
    case 0x58: i_0x58();                                    break;
    case 0x59: i_0x59();                                    break;
    case 0x5a: i_0x5a();                                    break;
    case 0x5b: i_0x5b();                                    break;
    case 0x5c: i_0x5c();                                    break;
    case 0x5d: i_0x5d();                                    break;
    case 0x5e: i_0x5e();                                    break;
    case 0x5f: i_0x5f();                                    break;
    case 0x60: i_0x60();                                    break;
    case 0x61: i_0x61();                                    break;
    case 0x62: i_0x62();                                    break;
    case 0x63: i_0x63();                                    break;
    case 0x64: i_0x64();                                    break;
    case 0x65: i_0x65();                                    break;
    case 0x66: i_0x66();                                    break;
    case 0x67: i_0x67();                                    break;
    case 0x68: i_0x68();                                    break;
    case 0x69: i_0x69();                                    break;
    case 0x6a: i_0x6a();                                    break;
    case 0x6b: i_0x6b();                                    break;
    case 0x6c: i_0x6c();                                    break;
    case 0x6d: i_0x6d();                                    break;
    case 0x6e: i_0x6e();                                    break;
    case 0x6f: i_0x6f();                                    break;
    case 0x70: i_0x70();                                    break;
    case 0x71: i_0x71();                                    break;
    case 0x72: i_0x72();                                    break;
    case 0x73: i_0x73();                                    break;
    case 0x74: i_0x74();                                    break;
    case 0x75: i_0x75();                                    break;
    case 0x76: i_0x76();                                    break;
    case 0x77: i_0x77();                                    break;
    case 0x78: i_0x78();                                    break;
    case 0x79: i_0x79();                                    break;
    case 0x7a: i_0x7a();                                    break;
    case 0x7b: i_0x7b();                                    break;
    case 0x7c: i_0x7c();                                    break;
    case 0x7d: i_0x7d();                                    break;
    case 0x7e: i_0x7e();                                    break;
    case 0x7f: i_0x7f();                                    break;
    case 0x80: i_0x80();                                    break;
    case 0x81: i_0x81();                                    break;
    case 0x82: i_0x82();                                    break;
    case 0x83: i_0x83();                                    break;
    case 0x84: i_0x84();                                    break;
    case 0x85: i_0x85();                                    break;
    case 0x86: i_0x86();                                    break;
    case 0x87: i_0x87();                                    break;
    case 0x88: i_0x88();                                    break;
    case 0x89: i_0x89();                                    break;
    case 0x8a: i_0x8a();                                    break;
    case 0x8b: i_0x8b();                                    break;
    case 0x8c: i_0x8c();                                    break;
    case 0x8d: i_0x8d();                                    break;
    case 0x8e: i_0x8e();                                    break;
    case 0x8f: i_0x8f();                                    break;
    case 0x90: i_0x90();                                    break;
    case 0x91: i_0x91();                                    break;
    case 0x92: i_0x92();                                    break;
    case 0x93: i_0x93();                                    break;
    case 0x94: i_0x94();                                    break;
    case 0x95: i_0x95();                                    break;
    case 0x96: i_0x96();                                    break;
    case 0x97: i_0x97();                                    break;
    case 0x98: i_0x98();                                    break;
    case 0x99: i_0x99();                                    break;
    case 0x9a: i_0x9a();                                    break;
    case 0x9b: i_0x9b();                                    break;
    case 0x9c: i_0x9c();                                    break;
    case 0x9d: i_0x9d();                                    break;
    case 0x9e: i_0x9e();                                    break;
    case 0x9f: i_0x9f();                                    break;
    case 0xa0: i_0xa0();                                    break;
    case 0xa1: i_0xa1();                                    break;
    case 0xa2: i_0xa2();                                    break;
    case 0xa3: i_0xa3();                                    break;
    case 0xa4: i_0xa4();                                    break;
    case 0xa5: i_0xa5();                                    break;
    case 0xa6: i_0xa6();                                    break;
    case 0xa7: i_0xa7();                                    break;
    case 0xa8: i_0xa8();                                    break;
    case 0xa9: i_0xa9();                                    break;
    case 0xaa: i_0xaa();                                    break;
    case 0xab: i_0xab();                                    break;
    case 0xac: i_0xac();                                    break;
    case 0xad: i_0xad();                                    break;
    case 0xae: i_0xae();                                    break;
    case 0xaf: i_0xaf();                                    break;
    case 0xb0: i_0xb0();                                    break;
    case 0xb1: i_0xb1();                                    break;
    case 0xb2: i_0xb2();                                    break;
    case 0xb3: i_0xb3();                                    break;
    case 0xb4: i_0xb4();                                    break;
    case 0xb5: i_0xb5();                                    break;
    case 0xb6: i_0xb6();                                    break;
    case 0xb7: i_0xb7();                                    break;
    case 0xb8: i_0xb8();                                    break;
    case 0xb9: i_0xb9();                                    break;
    case 0xba: i_0xba();                                    break;
    case 0xbb: i_0xbb();                                    break;
    case 0xbc: i_0xbc();                                    break;
    case 0xbd: i_0xbd();                                    break;
    case 0xbe: i_0xbe();                                    break;
    case 0xbf: i_0xbf();                                    break;
    case 0xc0: i_0xc0();                                    break;
    case 0xc1: i_0xc1();                                    break;
    case 0xc2: i_0xc2((m_currentOpcode&0x00ffff00)>>8);     break;
    case 0xc3: i_0xc3((m_currentOpcode&0x00ffff00)>>8);     break;
    case 0xc4: i_0xc4((m_currentOpcode&0x00ffff00)>>8);     break;
    case 0xc5: i_0xc5();                                    break;
    case 0xc6: i_0xc6((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0xc7: i_0xc7();                                    break;
    case 0xc8: i_0xc8();                                    break;
    case 0xc9: i_0xc9();                                    break;
    case 0xca: i_0xca((m_currentOpcode&0x00ffff00)>>8);     break;
    case 0xcb: i_0xcb();                                    break;
    case 0xcc: i_0xcc((m_currentOpcode&0x00ffff00)>>8);     break;
    case 0xcd: i_0xcd((m_currentOpcode&0x00ffff00)>>8);     break;
    case 0xce: i_0xce((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0xcf: i_0xcf();                                    break;
    case 0xd0: i_0xd0();                                    break;
    case 0xd1: i_0xd1();                                    break;
    case 0xd2: i_0xd2((m_currentOpcode&0x00ffff00)>>8);     break;
    case 0xd3: i_0xd3();                                    break;
    case 0xd4: i_0xd4((m_currentOpcode&0x00ffff00)>>8);     break;
    case 0xd5: i_0xd5();                                    break;
    case 0xd6: i_0xd6((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0xd7: i_0xd7();                                    break;
    case 0xd8: i_0xd8();                                    break;
    case 0xd9: i_0xd9();                                    break;
    case 0xda: i_0xda((m_currentOpcode&0x00ffff00)>>8);     break;
    case 0xdb: i_0xdb();                                    break;
    case 0xdc: i_0xdc((m_currentOpcode&0x00ffff00)>>8);     break;
    case 0xdd: i_0xdd();                                    break;
    case 0xde: i_0xde((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0xdf: i_0xdf();                                    break;
    case 0xe0: i_0xe0((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0xe1: i_0xe1();                                    break;
    case 0xe2: i_0xe2();                                    break;
    case 0xe3: i_0xe3();                                    break;
    case 0xe4: i_0xe4();                                    break;
    case 0xe5: i_0xe5();                                    break;
    case 0xe6: i_0xe6((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0xe7: i_0xe7();                                    break;
    case 0xe8: i_0xe8((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0xe9: i_0xe9();                                    break;
    case 0xea: i_0xea((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0xeb: i_0xeb();                                    break;
    case 0xec: i_0xec();                                    break;
    case 0xed: i_0xed();                                    break;
    case 0xee: i_0xee((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0xef: i_0xef();                                    break;
    case 0xf0: i_0xf0((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0xf1: i_0xf1();                                    break;
    case 0xf2: i_0xf2();                                    break;
    case 0xf3: i_0xf3();                                    break;
    case 0xf4: i_0xf4();                                    break;
    case 0xf5: i_0xf5();                                    break;
    case 0xf6: i_0xf6((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0xf7: i_0xf7();                                    break;
    case 0xf8: i_0xf8((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0xf9: i_0xf9();                                    break;
    case 0xfa: i_0xfa((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0xfb: i_0xfb();                                    break;
    case 0xfc: i_0xfc();                                    break;
    case 0xfd: i_0xfd();                                    break;
    case 0xfe: i_0xfe((m_currentOpcode&0x00ff0000)>>16);    break;
    case 0xff: i_0xff();                                    break;
    default:   IMPOSSIBLE();                                break;
    }
}

CPU::~CPU()
{
    delete m_registers;
}
