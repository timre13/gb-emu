#ifndef JOYPAD_H
#define JOYPAD_H

#include "Logger.h"
#include <SDL2/SDL.h>

#include <cassert>
#include <stdint.h>
#include <string>

class Joypad final
{
public:
    enum class Button
    {
        Up,
        Down,
        Left,
        Right,
        A,
        B,
        Select,
        Start,
        _Count,
    };

    static std::string buttonEnumToStr(Button btn)
    {
        static constexpr const char* const btnNames[(int)Button::_Count] = {
            "Up",
            "Down",
            "Left",
            "Right",
            "A",
            "B",
            "Select",
            "Start",
        };
        return btnNames[btnEnumToInt(btn)];
    }

    static int btnEnumToInt(Button btn)
    {
        const int val = (int)btn;
        assert(val >= 0 && val < (int)Button::_Count);
        return val;
    }

private:
    bool m_btnStates[(int)Button::_Count]{};
    bool m_isIntReq = false;

public:
    Joypad();

    inline bool isInterruptRequested() { return m_isIntReq; }
    inline void clearInterruptRequestedFlag() { m_isIntReq = false; }

    inline uint8_t isButtonPressed(Button btn) const
    {
        return m_btnStates[btnEnumToInt(btn)];
    }

    void onKeyPress(SDL_Keycode key);
    void onKeyRelease(SDL_Keycode key);

    inline void setBtnPressed(Button btn)
    {
        if (isButtonPressed(btn))
            return; // Exit if already set
        Logger::info("Pressed button: "+buttonEnumToStr(btn));
        m_btnStates[btnEnumToInt(btn)] = true;
        m_isIntReq = true;
    }

    inline void setBtnReleased(Button btn)
    {
        if (!isButtonPressed(btn))
            return; // Exit if already set
        Logger::info("Released button: "+buttonEnumToStr(btn));
        m_btnStates[btnEnumToInt(btn)] = false;
    }
};


static constexpr SDL_Keycode joypadKeyCodes[(int)Joypad::Button::_Count] = {
    SDLK_w,     // Up
    SDLK_s,     // Down
    SDLK_a,     // Left
    SDLK_d,     // Right
    SDLK_RIGHT, // Button A
    SDLK_LEFT,  // Button B
    SDLK_UP,    // Select
    SDLK_DOWN,  // Start
};

#endif // JOYPAD_H
