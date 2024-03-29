cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

link_libraries(SDL2 SDL2_ttf fontconfig)
include_directories(/usr/include/SDL2)

set(CMAKE_CXX_FLAGS "\
 -D_REENTRANT\
 -Wall\
 -Wextra\
 -Wpedantic\
 -g3\
 -ggdb\
")

project(GBEmu VERSION 1.0)

add_executable(gb-emu
    src/CPU.cpp
    src/CPU.h
    src/CartridgeReader.cpp
    src/CartridgeReader.h
    src/DebugWindow.cpp
    src/DebugWindow.h
    src/TextRenderer.cpp
    src/TextRenderer.h
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
    src/TileWindow.cpp
    src/TileWindow.h
    src/PPU.cpp
    src/PPU.h
    src/SerialViewer.cpp
    src/SerialViewer.h
    src/Joypad.cpp
    src/Joypad.h
    src/Timer.cpp
    src/Timer.h
)
