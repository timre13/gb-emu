

#ifndef BIT_MAGIC_H_
#define BIT_MAGIC_H_

#include <stdint.h>
#include <algorithm>


//################################ Bytes #################################

/*
 * Swaps all the bytes of an object byte by byte.
 */
template <typename T>
void swapBytes(T *value)
{
    //return ((value&0xff00)>>8) ||((value&0x00ff) << 8);
    std::reverse(reinterpret_cast<uint8_t*>(value),
                 reinterpret_cast<uint8_t*>(value)+sizeof(T));
}

/*
 * Returns the low (rightmost) byte of a 16-bit word.
 */
inline uint8_t getLowByte(uint16_t value)
{
    return value & 0x00ff;
}

/*
 * Returns the high (leftmost) byte of a 16-bit word.
 */
inline uint8_t getHighByte(uint16_t value)
{
    return (value & 0xff00) >> 8;
}

//#############################################################################


//################################ Half Carry #################################

/*
 * Returns a 1 byte unsigned integer
 * with bit 0 set if adding `a` and `b` would half carry.
 */
inline uint8_t wouldAddHalfCarry8(uint8_t a, uint8_t b)
{
    return (((a & 0x0f) + (b & 0x0f)) & 0b00010000) >> 4;
}

/*
 * Returns a 1 byte unsigned integer
 * with bit 0 set if subtracting `b` from `a` would half carry.
 */
inline uint8_t wouldSubHalfCarry8(uint8_t a, uint8_t b)
{
    return (((a & 0x0f) - (b & 0x0f)) & 0b00010000) >> 4;
}

/*
 * Returns a 1 byte unsigned integer
 * with bit 0 set if adding `a` and `b` would half carry.
 */
inline uint8_t wouldAddHalfCarry16(uint16_t a, uint16_t b)
{
    return (((a & 0x00ff) + (b & 0x00ff)) & 0b00000001'00000000) >> 8;
}

/*
 * Returns a 1 byte unsigned integer
 * with bit 0 set if subtracting `b` from `a` would half carry.
 */
inline uint8_t wouldSubHalfCarry16(uint16_t a, uint16_t b)
{
    return (((a & 0x00ff) - (b & 0x00ff)) & 0b00000001'00000000) >> 8;
}

//#############################################################################


//################################### Carry ###################################

/*
 * Returns a 1 byte unsigned integer
 * with bit 0 set if adding `a` and `b` would carry.
 */
inline uint8_t wouldAddCarry8(uint8_t a, uint8_t b)
{
    return ((static_cast<uint16_t>(a) + static_cast<uint16_t>(b)) & 0b00000001'00000000) >> 8;
}

/*
 * Returns a 1 byte unsigned integer
 * with bit 0 set if subtracting `b` from `a` would carry.
 */
inline uint8_t wouldSubCarry8(uint8_t a, uint8_t b)
{
    //return ((static_cast<uint16_t>(a) - static_cast<uint16_t>(b)) &
    //         0b00000001'00000000) >> 8;
    return b > a;
}

/*
 * Returns a 1 byte unsigned integer
 * with bit 0 set if adding `a` and `b` would carry.
 */
inline uint8_t wouldAddCarry16(uint16_t a, uint16_t b)
{
    return ((static_cast<uint32_t>(a) + static_cast<uint32_t>(b)) &
            0b00000000'00000001'00000000'00000000) >> 16;
}

/*
 * Returns a 1 byte unsigned integer
 * with bit 0 set if subtracting `b` from `a` would carry.
 */
inline uint8_t wouldSubCarry16(uint16_t a, uint16_t b)
{
    //return ((static_cast<uint32_t>(a) - static_cast<uint32_t>(b)) &
    //         0b00000000'00000001'00000000'00000000) >> 16;
    return b > a;
}

//#############################################################################



//################################# Bit #######################################

/*
 * Returns the Nth bit counted from right.
 */
template <typename T>
inline T getNthBit(T value, size_t index)
{
    return value & (1 << index);
}

//#############################################################################

#endif /* BIT_MAGIC_H_ */
