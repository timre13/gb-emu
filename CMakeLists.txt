cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

link_libraries(SDL2)
include_directories(/usr/include/SDL2)

set(CMAKE_CXX_FLAGS "-D_REENTRANT -Wall -Wextra -Wpedantic")

project(GBEmu VERSION 1.0)

add_executable(gb-emu
    src/CPU.cpp
    src/CPU.h
    src/CartridgeReader.cpp
    src/CartridgeReader.h
    src/DebugWindow.cpp
    src/DebugWindow.h
    src/FontManager.cpp
    src/FontManager.h
    src/GBEmulator.cpp
    src/GBEmulator.h
    src/Logger.cpp
    src/Logger.h
    src/Memory.cpp
    src/Memory.h
    src/Registers.cpp
    src/Registers.h
    src/bit_magic.h
    src/common.h
    src/config.h
    src/main.cpp
    src/opcode_sizes.h
    src/string_formatting.h
    src/opcode_names.h
)
