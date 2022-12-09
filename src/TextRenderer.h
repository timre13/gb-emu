#pragma once

#include "config.h"
#include "Logger.h"

#include <SDL2/SDL.h>

#include <array>
#include <cassert>
#include <memory>
#include <string>

#define TEXT_PADDING_PX 5

class Glyph
{
public:
    SDL_Texture* tex{};
    uint width{};
    uint height{};

    ~Glyph()
    {
        SDL_DestroyTexture(tex);
    }
};

#define PRINTABLE_CHAR_FIRST ('!')
#define PRINTABLE_CHAR_LAST  ('~')
#define PRINTABLE_CHAR_COUNT (PRINTABLE_CHAR_LAST-PRINTABLE_CHAR_FIRST+1)

class FontLoader
{
private:
    std::array<SDL_Surface*, PRINTABLE_CHAR_COUNT> m_surfaces{};

public:
    FontLoader(const std::string& fontNameOrPath);

    friend class TextRenderer;

    ~FontLoader();
};

class TextRenderer
{
private:
    std::array<std::unique_ptr<Glyph>, PRINTABLE_CHAR_COUNT> m_glyphs{};

    uint m_cursX{};
    uint m_cursY{};
    
    SDL_Renderer* m_rend{};

public:
    TextRenderer(SDL_Renderer* rend, FontLoader* loader);

    void endFrame()
    {
        m_cursX = TEXT_PADDING_PX;
        m_cursY = TEXT_PADDING_PX;
    }

    uint getCharW() const
    {
        return m_glyphs[0]->width;
    }

    uint getCharH() const
    {
        return m_glyphs[0]->height;
    }

    void renderText(const std::string &string);
};
