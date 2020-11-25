#include "SerialViewer.h"

#include "Logger.h"

SerialViewer::SerialViewer(int x, int y)
{
    m_window = SDL_CreateWindow("Serial Viewer", x, y, 0, 0, SDL_WINDOW_HIDDEN);
    if (!m_window) Logger::fatal("Failed to create window for Serial Viewer");

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    if (!m_renderer) Logger::fatal("Failed to create renderer for Serial Viewer");

    m_fontManager = new FontManager{m_window, &m_fontW, &m_fontH};
    SDL_SetWindowSize(m_window, m_winWChars*m_fontW+10, m_winHChars*m_fontH);

    SDL_SetRenderDrawColor(m_renderer, 220, 220, 220, 255);
    SDL_RenderClear(m_renderer);
}

void SerialViewer::updateText()
{
    SDL_SetRenderDrawColor(m_renderer, 220, 220, 220, 255);

    int cursorRow{};
    int cursorCol{};

    for (size_t i{}; i < m_buffer.length(); ++i)
    {
        char character{m_buffer[i]};

        switch (character)
        {
            case '\n':
                ++cursorRow;
                break;

            case '\r':
                cursorCol = 0;
                break;

            case '\t':
                cursorCol += 4;
                break;

            case '\v':
                cursorRow += 4;
                break;

            default:
                SDL_Texture *charTexture{m_fontManager->get(character)};

                SDL_Rect destRect{5+(int)cursorCol*m_fontW, cursorRow*m_fontH-m_scrollUpChars*m_fontH, m_fontW, m_fontH};
                SDL_RenderFillRect(m_renderer, &destRect);

                SDL_RenderCopy(m_renderer, charTexture, nullptr, &destRect);

                ++cursorCol;
                break;
        }

        if (cursorCol >= m_winWChars)
        {
            cursorCol = 0;
            ++cursorRow;
        }
    }

    // If the buffer is too long (in height)
    if (cursorRow > m_winHChars)
        // Scroll up the text
        m_scrollUpChars = cursorRow-m_winHChars+1;
}

SerialViewer::~SerialViewer()
{
    delete m_fontManager;

    SDL_DestroyWindow(m_window);
    SDL_DestroyRenderer(m_renderer);
}

