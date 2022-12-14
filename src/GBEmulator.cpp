#include "GBEmulator.h"
#include "Logger.h"
#include "string_formatting.h"
#include "opcode_names.h"

#include <SDL2/SDL_hints.h>
#include <SDL2/SDL_ttf.h>

#define FONT_NAME_OR_PATH "DejaVuSansMono"

//#define DEBUG_MODE
//#define SHOW_CARTRIDGE_INFO_MESSAGEBOX
//#define LOG_OPCODE
//#define USE_MAX_TEXTURE_SCALING_QUALITY
#define DELAY_BETWEEN_CYCLES_MS 0

GBEmulator::GBEmulator(const std::string &romFilename)
    : m_romFilename{romFilename}
{
    Logger::info("Starting emulator...");

    initGUI();
    initDebugWindow();
    initTileWindow();
    initSerialViewer();

#ifdef DEBUG_MODE
    toggleDebugWindow();
    toggleTileWindow();
    toggleSerialViewer();
#endif // DEBUG_MODE
    
    // We show the main window after the debug and tile window, so it pops up.
    SDL_ShowWindow(m_window);

    initHardware();

    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
    SDL_RenderClear(m_renderer);
    SDL_RenderPresent(m_renderer);

    toggleTileWindow();

    Logger::info("========== Emulator Started ==========");
}

void GBEmulator::initGUI()
{
    Logger::info("Initializing SDL2");

    if (SDL_Init(SDL_INIT_VIDEO))
        Logger::fatal("Failed to initialize SDL2");

    Logger::info("Initializing SDL2_ttf");

    if (TTF_Init())
        Logger::fatal("Failed to initialize SDL2_ttf");


    Logger::info("Creating window");

    m_window = SDL_CreateWindow(
            "Game Boy Emulator",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            TILE_MAP_DISPLAYED_TILES_PER_ROW*TILE_SIZE*PIXEL_SCALE, TILE_MAP_DISPLAYED_TILES_PER_COL*TILE_SIZE*PIXEL_SCALE,
            SDL_WINDOW_HIDDEN);

    if (m_window)
        Logger::info("Window created");
    else
        Logger::fatal("Failed to create window");

    m_windowId = SDL_GetWindowID(m_window);


    Logger::info("Creating renderer");

    m_renderer = SDL_CreateRenderer(
            m_window,
            -1,
            SDL_RENDERER_SOFTWARE);

    if (m_renderer)
        Logger::info("Renderer created");
    else
        Logger::fatal("Failed to create renderer");

    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);
    SDL_RenderPresent(m_renderer);

#ifdef USE_MAX_TEXTURE_SCALING_QUALITY
    // Set the best possible texture scaling.
    if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2"))
        if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
#endif

    m_fontLdr = new FontLoader{FONT_NAME_OR_PATH};
}

void GBEmulator::initDebugWindow()
{
    m_debugWindow = new DebugWindow{m_fontLdr, 20, 20};

    m_debugWindow->clearRenderer();
    m_debugWindow->updateRenderer();
}

void GBEmulator::initTileWindow()
{
    int winx{};
    SDL_GetWindowPosition(m_window, &winx, nullptr);
    int winw{};
    SDL_GetWindowSize(m_window, &winw, nullptr);
    m_tileWindow = new TileWindow{winx+winw+20, 20};

    m_tileWindow->updateRenderer();
}

void GBEmulator::initSerialViewer()
{
    m_serialViewer = new SerialViewer{m_fontLdr, 200, 0};

    m_serialViewer->updateRenderer();
}

void GBEmulator::initHardware()
{
    Logger::info("Initializing virtual hardware");

    m_cartridgeReader   = new CartridgeReader{m_romFilename};

    if (!m_cartridgeReader)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Game Boy Emulator - Error", "No cartridge", m_window);

        Logger::fatal("No cartridge (m_cartridgeReader is NULL)");
    }

    m_cartridgeInfo     = new CartridgeInfo{m_cartridgeReader->getCartridgeInfo()};

    m_joypad            = new Joypad;
    m_timer             = new Timer;
    m_memory            = new Memory{m_cartridgeInfo, m_serialViewer, m_joypad, m_timer};
    m_cpu               = new CPU{m_memory}; // the CPU needs to know about the memory to do the memory operations
    m_ppu               = new PPU{m_renderer, m_memory};

    showCartridgeInfo();

    if (m_cartridgeInfo->isCGBOnly)
    {
        Logger::error("ROM is CGB only");
        SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_ERROR,
                "ROM Error",
                "This ROM is for Game Boy Color. Sorry!",
                m_window);
        std::exit(1);
    }

    SDL_SetWindowTitle(m_window, ("Reading ROM: "+m_romFilename).c_str());

    m_cartridgeReader->loadRomToMemory(*m_memory);
    m_cartridgeReader->closeRomFile();

    SDL_SetWindowTitle(m_window, (std::string("Game Boy Emulator - ")+m_cartridgeInfo->title).c_str());
}

void GBEmulator::showCartridgeInfo()
{
    auto generateCartridgeInfoText{
        [this](int paddingWidth) {
            std::string textPadding(paddingWidth, ' ');
            return std::string("----- Cartridge info -----\n") +
                             textPadding + "Title:    "  +                      m_cartridgeInfo->title +                              '\n'  +
                             textPadding + "MBC type:     "  +                  toHexStr(m_cartridgeInfo->MBCType) +                  '\n'  +
                             textPadding + "ROM size: "  +                      toHexStr(m_cartridgeInfo->romSize) +                  " / " +
                                                                                std::to_string(m_cartridgeInfo->romSize) + " bytes" + '\n'  +
                             textPadding + "ROM banks: " +                      std::to_string(m_cartridgeInfo->romBanks) +           '\n'  +
                             textPadding + "RAM size: "  +                      toHexStr(m_cartridgeInfo->ramSize) +                  " / " +
                                                                                std::to_string(m_cartridgeInfo->ramSize) + " bytes" + '\n'  +
                             textPadding + "RAM banks: " +                      std::to_string(m_cartridgeInfo->ramBanks) +           '\n'  +
                             textPadding + "Is Super Game Boy supported? " +    (m_cartridgeInfo->isSGBSupported ? "yes" : "no") +    '\n'  +
                             textPadding + "Is Game Boy Color only? " +         (m_cartridgeInfo->isCGBOnly ? "yes" : "no") +         '\n'  +
                             textPadding + "Destination: " +                    (m_cartridgeInfo->isJapanese ? "Japan" : "EU/US") +   '\n'  +
                             textPadding + "Game version: " +                   std::to_string(m_cartridgeInfo->gameVersion+1);
        }
    };

    Logger::info(generateCartridgeInfoText(23));

#ifdef SHOW_CARTRIDGE_INFO_MESSAGEBOX
    SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_INFORMATION,
            "Cartridge information",
            generateCartridgeInfoText(0).c_str(), m_window);
#endif
}

void GBEmulator::startLoop()
{
    while (!m_isDone)
        emulateCycle();
}

void GBEmulator::emulateCycle()
{
    SDL_Event event;
    while (SDL_PollEvent(&event) && !m_isDone)
    {
        switch (event.type)
        {
        case SDL_QUIT:
            m_isDone = true;
            break;

        case SDL_KEYDOWN:
            m_joypad->onKeyPress(event.key.keysym.sym);
            switch (event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
                m_isDone = true;
                break;

            case SDLK_F11:
                if (event.window.windowID == m_windowId)
                    toggleDebugWindow();
                return;

            case SDLK_F12:
                if (event.window.windowID == m_windowId)
                    toggleTileWindow();
                return;

            case SDLK_F10:
                if (event.window.windowID == m_windowId)
                    toggleSerialViewer();
                return;
            }
            break;

        case SDL_KEYUP:
            m_joypad->onKeyRelease(event.key.keysym.sym);
            break;
        }
    }

    if (!m_isDone)
    {
        m_cpu->handleInterrupts();

        m_cpu->fetchOpcode();

#if defined(LOG_OPCODE) || defined(DEBUG_MODE)
        Logger::info("----- Cycle -----");
        Logger::info("PC: "+toHexStr(m_cpu->getRegisters()->getPC()));
        Logger::info("Opcode value: "+toHexStr(m_cpu->getCurrentOpcode()));
        Logger::info("Opcode name:  "+OpcodeNames::get(m_cpu->getCurrentOpcode() >> 24, m_cpu->isPrefixedOpcode()));
        Logger::info("Opcode size:  "+std::to_string(m_cpu->getCurrentOpcodeSize()));
#endif


#ifdef DEBUG_MODE
        waitForSpaceKey();
#endif // DEBUG_MODE

#if DELAY_BETWEEN_CYCLES_MS
        SDL_Delay(DELAY_BETWEEN_CYCLES_MS);
#endif

        int elapsedMCycles{};
        if (m_cpu->isPrefixedOpcode())
            elapsedMCycles = m_cpu->emulateCurrentPrefixedOpcode();
        else
            elapsedMCycles = m_cpu->emulateCurrentOpcode();

        for (int i{}; i < elapsedMCycles*4; ++i)
            m_timer->tick();
        for (int i{}; i < elapsedMCycles; ++i)
        {
            m_memory->tickDma();
        }

        // If the timer interrupt is requested
        if (m_timer->isInterruptRequested())
        {
            // Set the bit in IF
            m_memory->set(REGISTER_ADDR_IF, m_memory->get(REGISTER_ADDR_IF, false) | INTERRUPT_MASK_TIMER, false);
            m_timer->resetInterrupt();
        }

        if (m_joypad->isInterruptRequested())
        {
            Logger::info("Setting joypad bit in IF");
            // Set the bit in IF
            m_memory->set(REGISTER_ADDR_IF, m_memory->get(REGISTER_ADDR_IF, false) | INTERRUPT_MASK_JOYPAD, false);
            m_joypad->clearInterruptRequestedFlag();
        }

        for (int i{}; i < elapsedMCycles*4; ++i)
        {
            updateGraphics();

            if (m_memory->get(REGISTER_ADDR_LY) == 144 && m_ppu->isScanlineStart()) // Start of v-blank
            {
                SDL_SetWindowTitle(m_window, (std::string("Game Boy Emulator - ")
                            +m_cartridgeInfo->title+" - cycle "+std::to_string(m_cyclesDone)).c_str());
                SDL_RenderPresent(m_renderer);
                updateTileWindow();
                updateSerialViewer();
            }
        }

        if (m_joypad->isInterruptRequested())
        {
            Logger::info("Setting joypad bit in IF");
            // Set the bit in IF
            m_memory->set(REGISTER_ADDR_IF, m_memory->get(REGISTER_ADDR_IF, false) | INTERRUPT_MASK_JOYPAD, false);
            m_joypad->clearInterruptRequestedFlag();
        }

        updateDebugWindow();

        m_cpu->enableImaIfNeeded();
        m_cpu->stepPC();

        ++m_cyclesDone;
    }
}

void GBEmulator::updateGraphics()
{
    m_ppu->updateBackground();
}

void GBEmulator::waitForSpaceKey()
{
    SDL_Event event;

    while (true)
    {
        SDL_PollEvent(&event);

        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) break;

        SDL_Delay(16);
    }
}

void GBEmulator::updateDebugWindow()
{
    if (m_isDebugWindowShown)
    {
        m_debugWindow->clearRenderer();
        m_debugWindow->updateRegisterValues(m_cpu->getRegisters());
        m_debugWindow->updateOpcodeValue(m_cpu);
        m_debugWindow->updateMemoryValues(m_memory);
        m_debugWindow->updateRenderer();
    }
}

void GBEmulator::toggleDebugWindow()
{
    m_isDebugWindowShown = !m_isDebugWindowShown;

    if (m_isDebugWindowShown) m_debugWindow->show();
    else m_debugWindow->hide();

    updateDebugWindow();

    SDL_RaiseWindow(m_window);
}

void GBEmulator::updateTileWindow()
{
    if (m_isTileWindowShown)
    {
        m_tileWindow->updateTiles(m_ppu, m_memory->get(REGISTER_ADDR_LCDC, false));
        m_tileWindow->updateRenderer();
    }
}

void GBEmulator::toggleTileWindow()
{
    m_isTileWindowShown = !m_isTileWindowShown;

    if (m_isTileWindowShown) m_tileWindow->show();
    else m_tileWindow->hide();

    updateTileWindow();

    SDL_RaiseWindow(m_window);
}

void GBEmulator::updateSerialViewer()
{
    if (m_isSerialViewerShown)
    {
        m_serialViewer->clearRenderer();
        m_serialViewer->updateText();
        m_serialViewer->updateRenderer();
    }
}

void GBEmulator::toggleSerialViewer()
{
    m_isSerialViewerShown = !m_isSerialViewerShown;

    if (m_isSerialViewerShown) m_serialViewer->show();
    else m_serialViewer->hide();

    updateSerialViewer();

    SDL_RaiseWindow(m_window);
}

void GBEmulator::deinit()
{
    delete m_debugWindow;
    delete m_fontLdr;

    delete m_cpu;
    delete m_ppu;
    delete m_memory;
    delete m_cartridgeReader;
    delete m_cartridgeInfo;

    Logger::info("Cleaned up");

    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);

    SDL_Quit();
    TTF_Quit();

    Logger::info("SDL2 exited");
}

GBEmulator::~GBEmulator()
{
    deinit();

    Logger::info("========== Emulator exited ==========");
}
