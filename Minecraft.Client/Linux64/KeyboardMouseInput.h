#pragma once
#ifdef _LINUX64
#include <SDL2/SDL.h>

class KeyboardMouseInput
{
public:
    static const int MAX_KEYS = 512;
    static const int MOUSE_LEFT   = 0;
    static const int MOUSE_RIGHT  = 1;
    static const int MOUSE_MIDDLE = 2;
    static const int MAX_MOUSE_BUTTONS = 3;

    static const int KEY_FORWARD       = SDL_SCANCODE_W;
    static const int KEY_BACKWARD      = SDL_SCANCODE_S;
    static const int KEY_LEFT          = SDL_SCANCODE_A;
    static const int KEY_RIGHT         = SDL_SCANCODE_D;
    static const int KEY_JUMP          = SDL_SCANCODE_SPACE;
    static const int KEY_SNEAK         = SDL_SCANCODE_LSHIFT;
    static const int KEY_SPRINT        = SDL_SCANCODE_LCTRL;
    static const int KEY_INVENTORY     = SDL_SCANCODE_E;
    static const int KEY_DROP          = SDL_SCANCODE_Q;
    static const int KEY_CRAFTING      = SDL_SCANCODE_C;
    static const int KEY_CRAFTING_ALT  = SDL_SCANCODE_R;
    static const int KEY_CHAT          = SDL_SCANCODE_T;
    static const int KEY_CONFIRM       = SDL_SCANCODE_RETURN;
    static const int KEY_CANCEL        = SDL_SCANCODE_ESCAPE;
    static const int KEY_PAUSE         = SDL_SCANCODE_ESCAPE;
    static const int KEY_TOGGLE_HUD    = SDL_SCANCODE_F1;
    static const int KEY_SCREENSHOT    = SDL_SCANCODE_F2;
    static const int KEY_DEBUG_INFO    = SDL_SCANCODE_F3;
    static const int KEY_DEBUG_MENU    = SDL_SCANCODE_F4;
    static const int KEY_THIRD_PERSON  = SDL_SCANCODE_F5;
    static const int KEY_DEBUG_CONSOLE = SDL_SCANCODE_F6;
    static const int KEY_HOST_SETTINGS = SDL_SCANCODE_TAB;
    static const int KEY_FULLSCREEN    = SDL_SCANCODE_F11;

    void Init();
    void Tick();
    void ClearAllState();
    void OnKeyDown(int scancode);
    void OnKeyUp(int scancode);
    void OnMouseButtonDown(int button);
    void OnMouseButtonUp(int button);
    void OnMouseMove(int x, int y);
    void OnMouseWheel(int delta);
    void OnRawMouseDelta(int dx, int dy);

    bool IsKeyDown(int scancode) const;
    bool IsKeyPressed(int scancode) const;
    bool IsKeyReleased(int scancode) const;
    int  GetPressedKey() const;

    bool IsMouseButtonDown(int button) const;
    bool IsMouseButtonPressed(int button) const;
    bool IsMouseButtonReleased(int button) const;

    int  GetMouseX() const { return m_mouseX; }
    int  GetMouseY() const { return m_mouseY; }
    int  GetMouseDeltaX() const { return m_mouseDeltaX; }
    int  GetMouseDeltaY() const { return m_mouseDeltaY; }

    int  GetMouseWheel();
    int  PeekMouseWheel() const { return m_mouseWheelAccum; }
    void ConsumeMouseWheel() { if (m_mouseWheelAccum) m_mouseWheelConsumed = true; m_mouseWheelAccum = 0; }
    bool WasMouseWheelConsumed() const { return m_mouseWheelConsumed; }
    void ConsumeMouseDelta(float& dx, float& dy);

    void SetMouseGrabbed(bool grabbed);
    bool IsMouseGrabbed() const { return m_mouseGrabbed; }
    void SetCursorHiddenForUI(bool hidden);
    bool IsCursorHiddenForUI() const { return m_cursorHiddenForUI; }
    void SetWindowFocused(bool focused);
    bool IsWindowFocused() const { return m_windowFocused; }
    bool HasAnyInput() const { return m_hasInput; }
    void SetKBMActive(bool active) { m_kbmActive = active; }
    bool IsKBMActive() const { return m_kbmActive; }
    void SetScreenCursorHidden(bool hidden) { m_screenWantsCursorHidden = hidden; }
    bool IsScreenCursorHidden() const { return m_screenWantsCursorHidden; }

    void OnChar(wchar_t c);
    bool ConsumeChar(wchar_t& outChar);
    void ClearCharBuffer();

    float GetMoveX() const;
    float GetMoveY() const;
    float GetLookX(float sensitivity) const;
    float GetLookY(float sensitivity) const;

private:
    bool m_keyDown[MAX_KEYS]          = {};
    bool m_keyPressedAccum[MAX_KEYS]  = {};
    bool m_keyReleasedAccum[MAX_KEYS] = {};
    bool m_keyPressed[MAX_KEYS]       = {};
    bool m_keyReleased[MAX_KEYS]      = {};

    bool m_mouseButtonDown[MAX_MOUSE_BUTTONS]       = {};
    bool m_mouseBtnPressedAccum[MAX_MOUSE_BUTTONS]  = {};
    bool m_mouseBtnReleasedAccum[MAX_MOUSE_BUTTONS] = {};
    bool m_mouseBtnPressed[MAX_MOUSE_BUTTONS]       = {};
    bool m_mouseBtnReleased[MAX_MOUSE_BUTTONS]      = {};

    int  m_mouseX = 0, m_mouseY = 0;
    int  m_mouseDeltaX = 0, m_mouseDeltaY = 0;
    int  m_mouseDeltaAccumX = 0, m_mouseDeltaAccumY = 0;
    int  m_mouseWheelAccum = 0;
    bool m_mouseWheelConsumed = false;
    bool m_mouseGrabbed = false;
    bool m_cursorHiddenForUI = false;
    bool m_windowFocused = false;
    bool m_hasInput = false;
    bool m_kbmActive = true;
    bool m_screenWantsCursorHidden = false;
    int  m_pressedKey = -1;

    static const int CHAR_BUFFER_SIZE = 32;
    wchar_t m_charBuffer[CHAR_BUFFER_SIZE] = {};
    int m_charBufferHead = 0;
    int m_charBufferTail = 0;
};

extern KeyboardMouseInput g_KBMInput;
#endif // _LINUX64
