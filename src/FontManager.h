#ifndef FONTMANAGER_H_
#define FONTMANAGER_H_

#include "config.h"

#include <SDL2/SDL.h>

#include <array>

class FontManager
{
private:
    std::array<SDL_Texture*, 256> m_characters {};

public:
    FontManager(SDL_Window *window, int *charWidthOut, int *charHeightOut);

    inline SDL_Texture* get(unsigned char charCode) { return m_characters[charCode]; }

    ~FontManager();
};

#endif /* FONTMANAGER_H_ */
