#include "FontManager.h"
#include "Logger.h"

#include <cassert>
#include <string>

static SDL_Texture* loadImage(const std::string &filename, SDL_Window *window, int *wOut, int *hOut)
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

    if (wOut) *wOut = imageSurface->w;
    if (hOut) *hOut = imageSurface->h;

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

FontManager::FontManager(SDL_Window *window, int *charWidthOut, int *charHeightOut)
{
    Logger::info("Loading font");

    for (int i{}; i < 256; ++i)
    {
        m_characters[i] = loadImage("assets/font/character_"+std::to_string(i)+".bmp", window, charWidthOut, charHeightOut);
        SDL_SetTextureColorMod(m_characters[i], 0, 0, 0);
    }
}

FontManager::~FontManager()
{
    for (int i{}; i < 256; ++i)
        SDL_DestroyTexture(m_characters[i]);

    Logger::info("Font free'd");
}

