#ifndef JOYPAD_H
#define JOYPAD_H

#include <SDL2/SDL.h>

#include <stdint.h>

#define JOYPAD_SCANCODE_UP    SDL_SCANCODE_W
#define JOYPAD_SCANCODE_DOWN  SDL_SCANCODE_S
#define JOYPAD_SCANCODE_LEFT  SDL_SCANCODE_A
#define JOYPAD_SCANCODE_RIGHT SDL_SCANCODE_D

#define JOYPAD_SCANCODE_A      SDL_SCANCODE_RIGHT
#define JOYPAD_SCANCODE_B      SDL_SCANCODE_LEFT
#define JOYPAD_SCANCODE_SELECT SDL_SCANCODE_UP
#define JOYPAD_SCANCODE_START  SDL_SCANCODE_DOWN

class Joypad final
{
private:
    const Uint8* m_keyboardState{};

public:
    Joypad();

    inline void updateState() { SDL_PumpEvents(); }

    inline uint8_t isUpButtonPressed()    const { return m_keyboardState[JOYPAD_SCANCODE_UP]; }
    inline uint8_t isDownButtonPressed()  const { return m_keyboardState[JOYPAD_SCANCODE_DOWN]; }
    inline uint8_t isLeftButtonPressed()  const { return m_keyboardState[JOYPAD_SCANCODE_LEFT]; }
    inline uint8_t isRightButtonPressed() const { return m_keyboardState[JOYPAD_SCANCODE_RIGHT]; }

    inline uint8_t isAButtonPressed()      const { return m_keyboardState[JOYPAD_SCANCODE_A]; }
    inline uint8_t isBButtonPressed()      const { return m_keyboardState[JOYPAD_SCANCODE_B]; }
    inline uint8_t isSelectButtonPressed() const { return m_keyboardState[JOYPAD_SCANCODE_SELECT]; }
    inline uint8_t isStartButtonPressed()  const { return m_keyboardState[JOYPAD_SCANCODE_START]; }
};

#endif // JOYPAD_H
