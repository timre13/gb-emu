#include "TextRenderer.h"
#include "Logger.h"
#include "string_formatting.h"

#include <SDL2/SDL_ttf.h>

#include <cassert>
#include <string>

static SDL_Surface* loadGlyphSurface(TTF_Font* font, Uint16 charCode)
{
    SDL_Surface* surf = TTF_RenderGlyph32_Blended(font, charCode, SDL_Color{0, 0, 0, 255});
    if (!surf)
    {
        Logger::fatal("Failed to load character "+toHexStr(charCode));
    }

    return surf;
}

FontLoader::FontLoader(const std::string& fontNameOrPath)
{
    Logger::info("Loading font...");

    const std::string fontPath = fontNameOrPath;

    TTF_Font* font = TTF_OpenFont(fontPath.c_str(), 14);
    if (!font)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "",
                ("Failed to load font: "+fontPath+": "+TTF_GetError()).c_str(), nullptr);
        return;
    }

    for (Uint16 i{}; i < PRINTABLE_CHAR_COUNT; ++i)
    {
        m_surfaces[i] = loadGlyphSurface(font, PRINTABLE_CHAR_FIRST+i);
    }

    TTF_CloseFont(font);
}

FontLoader::~FontLoader()
{
    for (SDL_Surface* surf : m_surfaces)
    {
        SDL_FreeSurface(surf);
    }
    Logger::info("Freed font surfaces");
}

static Glyph* surfaceToGlyph(SDL_Surface* surf, SDL_Renderer* rend)
{
    assert(surf);
    assert(rend);

    const int width = surf->w;
    const int height = surf->h;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surf);
    if (!tex)
    {
        Logger::fatal("Failed to create texture from surface for glyph");
    }

    Glyph* glyph = new Glyph{};
    glyph->tex = tex;
    tex = nullptr;
    glyph->width = width;
    glyph->height = height;
    return glyph;
}

TextRenderer::TextRenderer(SDL_Renderer* rend, FontLoader* loader)
{
    for (size_t i{}; i < loader->m_surfaces.size(); ++i)
    {
        m_glyphs[i] = std::unique_ptr<Glyph>(surfaceToGlyph(loader->m_surfaces[i], rend));
    }

    m_rend = rend;
}

void TextRenderer::renderText(const std::string &string)
{
    for (size_t i{}; i < string.length(); ++i)
    {
        const char charCode = string[i];

        switch (charCode)
        {
        case ' ':
            m_cursX += getCharW();
            continue;

        case '\n':
            m_cursX = TEXT_PADDING_PX;
            m_cursY += getCharH();
            continue;

        case '\r':
            m_cursX = TEXT_PADDING_PX;
            continue;

        case '\t':
            m_cursX += getCharW()*4;
            break;

        case '\v':
            m_cursY += getCharH()*4;
            break;

        default: ;
        }

        assert(std::isprint(charCode));

        const size_t glyphIndex = charCode-PRINTABLE_CHAR_FIRST;
        assert(glyphIndex < m_glyphs.size());

        const Glyph* const glyph = m_glyphs[glyphIndex].get();
        SDL_Texture* const charTexture{glyph->tex};

        const SDL_Rect destRect{(int)m_cursX, (int)m_cursY, (int)glyph->width, (int)glyph->height};
        m_cursX += glyph->width;
        SDL_RenderCopy(m_rend, charTexture, nullptr, &destRect);
    }
}
