#include "SerialViewer.h"

#include "Logger.h"

SerialViewer::SerialViewer(FontLoader* fontLdr, int x, int y)
{
    m_window = SDL_CreateWindow("Serial Viewer", x, y, 0, 0, SDL_WINDOW_HIDDEN);
    if (!m_window) Logger::fatal("Failed to create window for Serial Viewer");

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    if (!m_renderer) Logger::fatal("Failed to create renderer for Serial Viewer");

    m_textRend = std::unique_ptr<TextRenderer>(new TextRenderer{m_renderer, fontLdr});
    SDL_SetWindowSize(m_window, winWChars*m_textRend->getCharW()+TEXT_PADDING_PX*2, winHChars*m_textRend->getCharH()+TEXT_PADDING_PX*2);
}

void SerialViewer::updateText()
{
    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);

    // TODO: Wrap characters

    m_textRend->renderText(m_buffer);
}

SerialViewer::~SerialViewer()
{
    SDL_DestroyWindow(m_window);
    SDL_DestroyRenderer(m_renderer);
}

