#include "FontManager.h"
#include "Logger.h"

#include <cassert>
#include <string>

static SDL_Texture* loadImage(const std::string &filename, SDL_Window *window, int *w, int *h)
{
    SDL_Surface *imageSurface{SDL_LoadBMP(filename.c_str())};

    if (!imageSurface)
    {
        SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_ERROR,
                "Error loading image",
                (std::string("Failed to load character image: ") + SDL_GetError()).c_str(), window);

        Logger::fatal("Failed to load character image: "+std::string(SDL_GetError()));
    }

    *w = imageSurface->w;
    *h = imageSurface->h;

    SDL_Texture *imageTexture{SDL_CreateTextureFromSurface(SDL_GetRenderer(window), imageSurface)};

    if (!imageTexture)
    {
        SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_ERROR,
                "Error loading image",
                (std::string("Failed to load character image: ") + SDL_GetError()).c_str(), window);

        Logger::fatal("Failed to load character image: "+std::string(SDL_GetError()));
    }

    SDL_FreeSurface(imageSurface);

    return imageTexture;
}

FontManager::FontManager(SDL_Window *window)
{
    Logger::info("Loading font");

    for (int i{}; i < 256; ++i)
    {
        int w, h;
        auto texture = loadImage("assets/font/character_"+std::to_string(i)+".bmp", window, &w, &h);
        m_characters[i] = new FontCharacter{texture, w, h};
    }

    for (int i{}; i < 256; ++i)
        SDL_SetTextureColorMod(m_characters[i]->getTexture(), 0, 0, 0);
}

FontCharacter* FontManager::get(int charCode)
{
    assert(charCode < 256);
    assert(charCode >= 0);

    return m_characters[charCode];
}

FontManager::~FontManager()
{
    for (int i{}; i < 256; ++i)
        delete m_characters[i];

    Logger::info("Font free'd");
}

