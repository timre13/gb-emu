#ifndef FONTMANAGER_H_
#define FONTMANAGER_H_

#include "config.h"

#include <SDL2/SDL.h>

#include <array>

class FontCharacter final
{
private:
    SDL_Texture *m_texture;
    int m_w;
    int m_h;

public:
    inline FontCharacter(SDL_Texture *texture, int w, int h)
        : m_texture{texture}, m_w{w}, m_h{h}
    {
    }

    inline SDL_Texture *getTexture() { return m_texture; }
    inline int getW() const { return m_w; }
    inline int getH() const { return m_h; }

    inline ~FontCharacter() { SDL_DestroyTexture(m_texture); }
};

class FontManager
{
private:
    std::array<FontCharacter*, 256> m_characters {};

public:
    FontManager(SDL_Window *window);

    FontCharacter* get(int charCode);

    ~FontManager();
};

#endif /* FONTMANAGER_H_ */
