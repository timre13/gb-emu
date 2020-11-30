#include "Joypad.h"

Joypad::Joypad()
    : m_keyboardState{SDL_GetKeyboardState(nullptr)}
{
}
