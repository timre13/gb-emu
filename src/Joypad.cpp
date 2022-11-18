#include "Joypad.h"

Joypad::Joypad()
{
}

void Joypad::onKeyPress(SDL_Keycode key)
{
    for (int i{}; i < (int)Joypad::Button::_Count; ++i)
    {
        if (key == joypadKeyCodes[i])
        {
            setBtnPressed((Joypad::Button)i);
            break;
        }
    }
}

void Joypad::onKeyRelease(SDL_Keycode key)
{
    for (int i{}; i < (int)Joypad::Button::_Count; ++i)
    {
        if (key == joypadKeyCodes[i])
        {
            setBtnReleased((Joypad::Button)i);
            break;
        }
    }
}
