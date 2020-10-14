#ifndef TO_HEX_H
#define TO_HEX_H

#include <string>
#include <iomanip>
#include <sstream>
#include <bitset>
#include <cassert>

template<typename T>
std::string toHexStr(T value, size_t width=sizeof(T)/2, bool prefix=true)
{
    std::stringstream ss;

    ss << std::setfill('0') << std::setw(width) << std::hex << (value | 0);

    if (prefix)
        return "0x"+ss.str();

    return ss.str();
}

template <typename T>
std::string toBinStr(T value, size_t width=sizeof(T)*8, bool prefix=true)
{
    std::stringstream ss;

    switch (width)
    {
    case    1: ss << std::setfill('0') << std::setw(width) << std::bitset<   1>(value).to_string(); break;
    case    2: ss << std::setfill('0') << std::setw(width) << std::bitset<   2>(value).to_string(); break;
    case    4: ss << std::setfill('0') << std::setw(width) << std::bitset<   4>(value).to_string(); break;
    case    8: ss << std::setfill('0') << std::setw(width) << std::bitset<   8>(value).to_string(); break;
    case   16: ss << std::setfill('0') << std::setw(width) << std::bitset<  16>(value).to_string(); break;
    case   32: ss << std::setfill('0') << std::setw(width) << std::bitset<  32>(value).to_string(); break;
    case   64: ss << std::setfill('0') << std::setw(width) << std::bitset<  64>(value).to_string(); break;
    case  128: ss << std::setfill('0') << std::setw(width) << std::bitset< 128>(value).to_string(); break;
    case  256: ss << std::setfill('0') << std::setw(width) << std::bitset< 256>(value).to_string(); break;
    case  512: ss << std::setfill('0') << std::setw(width) << std::bitset< 512>(value).to_string(); break;
    case 1024: ss << std::setfill('0') << std::setw(width) << std::bitset<1024>(value).to_string(); break;
    default: assert(0 && "This width cannot be used.");
    }

    if (prefix)
        return "0b"+ss.str();

    return ss.str();
}

inline std::string alignRight(const std::string &str, char fillWith, int width)
{
    std::stringstream ss;

    ss << std::setfill(fillWith) << std::setw(width) << str;

    return ss.str();
}


#endif /* TO_HEX_H */
