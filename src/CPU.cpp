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

int CPU::emulateCurrentOpcode()
{
    m_wasJump = false;

    switch (m_currentOpcode >> 24)
    {
    case 0x00: return i_0x00();
    case 0x01: return i_0x01((m_currentOpcode&0x00ffff00)>>8);
    case 0x02: return i_0x02();
    case 0x03: return i_0x03();
    case 0x04: return i_0x04();
    case 0x05: return i_0x05();
    case 0x06: return i_0x06((m_currentOpcode&0x00ff0000)>>16);
    case 0x07: return i_0x07();
    case 0x08: return i_0x08((m_currentOpcode&0x00ffff00)>>8);
    case 0x09: return i_0x09();
    case 0x0a: return i_0x0a();
    case 0x0b: return i_0x0b();
    case 0x0c: return i_0x0c();
    case 0x0d: return i_0x0d();
    case 0x0e: return i_0x0e((m_currentOpcode&0x00ff0000)>>16);
    case 0x0f: return i_0x0f();
    case 0x10: return i_0x10();
    case 0x11: return i_0x11((m_currentOpcode&0x00ffff00)>>8);
    case 0x12: return i_0x12();
    case 0x13: return i_0x13();
    case 0x14: return i_0x14();
    case 0x15: return i_0x15();
    case 0x16: return i_0x16((m_currentOpcode&0x00ff0000)>>16);
    case 0x17: return i_0x17();
    case 0x18: return i_0x18((m_currentOpcode&0x00ff0000)>>16);
    case 0x19: return i_0x19();
    case 0x1a: return i_0x1a();
    case 0x1b: return i_0x1b();
    case 0x1c: return i_0x1c();
    case 0x1d: return i_0x1d();
    case 0x1e: return i_0x1e((m_currentOpcode&0x00ff0000)>>16);
    case 0x1f: return i_0x1f();
    case 0x20: return i_0x20((m_currentOpcode&0x00ff0000)>>16);
    case 0x21: return i_0x21((m_currentOpcode&0x00ffff00)>>8);
    case 0x22: return i_0x22();
    case 0x23: return i_0x23();
    case 0x24: return i_0x24();
    case 0x25: return i_0x25();
    case 0x26: return i_0x26((m_currentOpcode&0x00ff0000)>>16);
    case 0x27: return i_0x27();
    case 0x28: return i_0x28((m_currentOpcode&0x00ff0000)>>16);
    case 0x29: return i_0x29();
    case 0x2a: return i_0x2a();
    case 0x2b: return i_0x2b();
    case 0x2c: return i_0x2c();
    case 0x2d: return i_0x2d();
    case 0x2e: return i_0x2e((m_currentOpcode&0x00ff0000)>>16);
    case 0x2f: return i_0x2f();
    case 0x30: return i_0x30((m_currentOpcode&0x00ff0000)>>16);
    case 0x31: return i_0x31((m_currentOpcode&0x00ffff00)>>8);
    case 0x32: return i_0x32();
    case 0x33: return i_0x33();
    case 0x34: return i_0x34();
    case 0x35: return i_0x35();
    case 0x36: return i_0x36((m_currentOpcode&0x00ff0000)>>16);
    case 0x37: return i_0x37();
    case 0x38: return i_0x38((m_currentOpcode&0x00ff0000)>>16);
    case 0x39: return i_0x39();
    case 0x3a: return i_0x3a();
    case 0x3b: return i_0x3b();
    case 0x3c: return i_0x3c();
    case 0x3d: return i_0x3d();
    case 0x3e: return i_0x3e((m_currentOpcode&0x00ff0000)>>16);
    case 0x3f: return i_0x3f();
    case 0x40: return i_0x40();
    case 0x41: return i_0x41();
    case 0x42: return i_0x42();
    case 0x43: return i_0x43();
    case 0x44: return i_0x44();
    case 0x45: return i_0x45();
    case 0x46: return i_0x46();
    case 0x47: return i_0x47();
    case 0x48: return i_0x48();
    case 0x49: return i_0x49();
    case 0x4a: return i_0x4a();
    case 0x4b: return i_0x4b();
    case 0x4c: return i_0x4c();
    case 0x4d: return i_0x4d();
    case 0x4e: return i_0x4e();
    case 0x4f: return i_0x4f();
    case 0x50: return i_0x50();
    case 0x51: return i_0x51();
    case 0x52: return i_0x52();
    case 0x53: return i_0x53();
    case 0x54: return i_0x54();
    case 0x55: return i_0x55();
    case 0x56: return i_0x56();
    case 0x57: return i_0x57();
    case 0x58: return i_0x58();
    case 0x59: return i_0x59();
    case 0x5a: return i_0x5a();
    case 0x5b: return i_0x5b();
    case 0x5c: return i_0x5c();
    case 0x5d: return i_0x5d();
    case 0x5e: return i_0x5e();
    case 0x5f: return i_0x5f();
    case 0x60: return i_0x60();
    case 0x61: return i_0x61();
    case 0x62: return i_0x62();
    case 0x63: return i_0x63();
    case 0x64: return i_0x64();
    case 0x65: return i_0x65();
    case 0x66: return i_0x66();
    case 0x67: return i_0x67();
    case 0x68: return i_0x68();
    case 0x69: return i_0x69();
    case 0x6a: return i_0x6a();
    case 0x6b: return i_0x6b();
    case 0x6c: return i_0x6c();
    case 0x6d: return i_0x6d();
    case 0x6e: return i_0x6e();
    case 0x6f: return i_0x6f();
    case 0x70: return i_0x70();
    case 0x71: return i_0x71();
    case 0x72: return i_0x72();
    case 0x73: return i_0x73();
    case 0x74: return i_0x74();
    case 0x75: return i_0x75();
    case 0x76: return i_0x76();
    case 0x77: return i_0x77();
    case 0x78: return i_0x78();
    case 0x79: return i_0x79();
    case 0x7a: return i_0x7a();
    case 0x7b: return i_0x7b();
    case 0x7c: return i_0x7c();
    case 0x7d: return i_0x7d();
    case 0x7e: return i_0x7e();
    case 0x7f: return i_0x7f();
    case 0x80: return i_0x80();
    case 0x81: return i_0x81();
    case 0x82: return i_0x82();
    case 0x83: return i_0x83();
    case 0x84: return i_0x84();
    case 0x85: return i_0x85();
    case 0x86: return i_0x86();
    case 0x87: return i_0x87();
    case 0x88: return i_0x88();
    case 0x89: return i_0x89();
    case 0x8a: return i_0x8a();
    case 0x8b: return i_0x8b();
    case 0x8c: return i_0x8c();
    case 0x8d: return i_0x8d();
    case 0x8e: return i_0x8e();
    case 0x8f: return i_0x8f();
    case 0x90: return i_0x90();
    case 0x91: return i_0x91();
    case 0x92: return i_0x92();
    case 0x93: return i_0x93();
    case 0x94: return i_0x94();
    case 0x95: return i_0x95();
    case 0x96: return i_0x96();
    case 0x97: return i_0x97();
    case 0x98: return i_0x98();
    case 0x99: return i_0x99();
    case 0x9a: return i_0x9a();
    case 0x9b: return i_0x9b();
    case 0x9c: return i_0x9c();
    case 0x9d: return i_0x9d();
    case 0x9e: return i_0x9e();
    case 0x9f: return i_0x9f();
    case 0xa0: return i_0xa0();
    case 0xa1: return i_0xa1();
    case 0xa2: return i_0xa2();
    case 0xa3: return i_0xa3();
    case 0xa4: return i_0xa4();
    case 0xa5: return i_0xa5();
    case 0xa6: return i_0xa6();
    case 0xa7: return i_0xa7();
    case 0xa8: return i_0xa8();
    case 0xa9: return i_0xa9();
    case 0xaa: return i_0xaa();
    case 0xab: return i_0xab();
    case 0xac: return i_0xac();
    case 0xad: return i_0xad();
    case 0xae: return i_0xae();
    case 0xaf: return i_0xaf();
    case 0xb0: return i_0xb0();
    case 0xb1: return i_0xb1();
    case 0xb2: return i_0xb2();
    case 0xb3: return i_0xb3();
    case 0xb4: return i_0xb4();
    case 0xb5: return i_0xb5();
    case 0xb6: return i_0xb6();
    case 0xb7: return i_0xb7();
    case 0xb8: return i_0xb8();
    case 0xb9: return i_0xb9();
    case 0xba: return i_0xba();
    case 0xbb: return i_0xbb();
    case 0xbc: return i_0xbc();
    case 0xbd: return i_0xbd();
    case 0xbe: return i_0xbe();
    case 0xbf: return i_0xbf();
    case 0xc0: return i_0xc0();
    case 0xc1: return i_0xc1();
    case 0xc2: return i_0xc2((m_currentOpcode&0x00ffff00)>>8);
    case 0xc3: return i_0xc3((m_currentOpcode&0x00ffff00)>>8);
    case 0xc4: return i_0xc4((m_currentOpcode&0x00ffff00)>>8);
    case 0xc5: return i_0xc5();
    case 0xc6: return i_0xc6((m_currentOpcode&0x00ff0000)>>16);
    case 0xc7: return i_0xc7();
    case 0xc8: return i_0xc8();
    case 0xc9: return i_0xc9();
    case 0xca: return i_0xca((m_currentOpcode&0x00ffff00)>>8);
    case 0xcb: return i_0xcb();
    case 0xcc: return i_0xcc((m_currentOpcode&0x00ffff00)>>8);
    case 0xcd: return i_0xcd((m_currentOpcode&0x00ffff00)>>8);
    case 0xce: return i_0xce((m_currentOpcode&0x00ff0000)>>16);
    case 0xcf: return i_0xcf();
    case 0xd0: return i_0xd0();
    case 0xd1: return i_0xd1();
    case 0xd2: return i_0xd2((m_currentOpcode&0x00ffff00)>>8);
    case 0xd3: return i_0xd3();
    case 0xd4: return i_0xd4((m_currentOpcode&0x00ffff00)>>8);
    case 0xd5: return i_0xd5();
    case 0xd6: return i_0xd6((m_currentOpcode&0x00ff0000)>>16);
    case 0xd7: return i_0xd7();
    case 0xd8: return i_0xd8();
    case 0xd9: return i_0xd9();
    case 0xda: return i_0xda((m_currentOpcode&0x00ffff00)>>8);
    case 0xdb: return i_0xdb();
    case 0xdc: return i_0xdc((m_currentOpcode&0x00ffff00)>>8);
    case 0xdd: return i_0xdd();
    case 0xde: return i_0xde((m_currentOpcode&0x00ff0000)>>16);
    case 0xdf: return i_0xdf();
    case 0xe0: return i_0xe0((m_currentOpcode&0x00ff0000)>>16);
    case 0xe1: return i_0xe1();
    case 0xe2: return i_0xe2();
    case 0xe3: return i_0xe3();
    case 0xe4: return i_0xe4();
    case 0xe5: return i_0xe5();
    case 0xe6: return i_0xe6((m_currentOpcode&0x00ff0000)>>16);
    case 0xe7: return i_0xe7();
    case 0xe8: return i_0xe8((m_currentOpcode&0x00ff0000)>>16);
    case 0xe9: return i_0xe9();
    case 0xea: return i_0xea((m_currentOpcode&0x00ff0000)>>16);
    case 0xeb: return i_0xeb();
    case 0xec: return i_0xec();
    case 0xed: return i_0xed();
    case 0xee: return i_0xee((m_currentOpcode&0x00ff0000)>>16);
    case 0xef: return i_0xef();
    case 0xf0: return i_0xf0((m_currentOpcode&0x00ff0000)>>16);
    case 0xf1: return i_0xf1();
    case 0xf2: return i_0xf2();
    case 0xf3: return i_0xf3();
    case 0xf4: return i_0xf4();
    case 0xf5: return i_0xf5();
    case 0xf6: return i_0xf6((m_currentOpcode&0x00ff0000)>>16);
    case 0xf7: return i_0xf7();
    case 0xf8: return i_0xf8((m_currentOpcode&0x00ff0000)>>16);
    case 0xf9: return i_0xf9();
    case 0xfa: return i_0xfa((m_currentOpcode&0x00ff0000)>>16);
    case 0xfb: return i_0xfb();
    case 0xfc: return i_0xfc();
    case 0xfd: return i_0xfd();
    case 0xfe: return i_0xfe((m_currentOpcode&0x00ff0000)>>16);
    case 0xff: return i_0xff();
    default:   return -1;
    }
}

CPU::~CPU()
{
    delete m_registers;
}
