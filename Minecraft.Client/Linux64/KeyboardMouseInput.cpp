#ifdef _LINUX64
#include "stdafx.h"
#include "Linux64/KeyboardMouseInput.h"
#include <SDL2/SDL.h>
#include <cstring>

KeyboardMouseInput g_KBMInput;

void KeyboardMouseInput::Init()    { ClearAllState(); }

void KeyboardMouseInput::ClearAllState()
{
    memset(m_keyDown,          0, sizeof(m_keyDown));
    memset(m_keyPressedAccum,  0, sizeof(m_keyPressedAccum));
    memset(m_keyReleasedAccum, 0, sizeof(m_keyReleasedAccum));
    memset(m_keyPressed,       0, sizeof(m_keyPressed));
    memset(m_keyReleased,      0, sizeof(m_keyReleased));
    memset(m_mouseButtonDown,       0, sizeof(m_mouseButtonDown));
    memset(m_mouseBtnPressedAccum,  0, sizeof(m_mouseBtnPressedAccum));
    memset(m_mouseBtnReleasedAccum, 0, sizeof(m_mouseBtnReleasedAccum));
    memset(m_mouseBtnPressed,       0, sizeof(m_mouseBtnPressed));
    memset(m_mouseBtnReleased,      0, sizeof(m_mouseBtnReleased));
    m_mouseX = m_mouseY = 0;
    m_mouseDeltaX = m_mouseDeltaY = 0;
    m_mouseDeltaAccumX = m_mouseDeltaAccumY = 0;
    m_mouseWheelAccum = 0;
    m_mouseWheelConsumed = false;
    m_hasInput = false;
    m_pressedKey = -1;
    ClearCharBuffer();
}

void KeyboardMouseInput::Tick()
{
    memcpy(m_keyPressed,  m_keyPressedAccum,  sizeof(m_keyPressed));
    memcpy(m_keyReleased, m_keyReleasedAccum, sizeof(m_keyReleased));
    memset(m_keyPressedAccum,  0, sizeof(m_keyPressedAccum));
    memset(m_keyReleasedAccum, 0, sizeof(m_keyReleasedAccum));
    memcpy(m_mouseBtnPressed,  m_mouseBtnPressedAccum,  sizeof(m_mouseBtnPressed));
    memcpy(m_mouseBtnReleased, m_mouseBtnReleasedAccum, sizeof(m_mouseBtnReleased));
    memset(m_mouseBtnPressedAccum,  0, sizeof(m_mouseBtnPressedAccum));
    memset(m_mouseBtnReleasedAccum, 0, sizeof(m_mouseBtnReleasedAccum));
    m_mouseDeltaX = m_mouseDeltaAccumX;
    m_mouseDeltaY = m_mouseDeltaAccumY;
    m_mouseDeltaAccumX = 0;
    m_mouseDeltaAccumY = 0;
    m_mouseWheelConsumed = false;
    m_pressedKey = -1;
    m_hasInput = false;
    for (int i = 0; i < MAX_KEYS && !m_hasInput; i++)
        if (m_keyPressed[i]) m_hasInput = true;
    for (int i = 0; i < MAX_MOUSE_BUTTONS && !m_hasInput; i++)
        if (m_mouseBtnPressed[i]) m_hasInput = true;
    if (m_mouseDeltaX || m_mouseDeltaY || m_mouseWheelAccum) m_hasInput = true;
}

void KeyboardMouseInput::OnKeyDown(int sc) {
    if (sc < 0 || sc >= MAX_KEYS) return;
    if (!m_keyDown[sc]) { m_keyPressedAccum[sc] = true; m_pressedKey = sc; }
    m_keyDown[sc] = true;
}
void KeyboardMouseInput::OnKeyUp(int sc) {
    if (sc < 0 || sc >= MAX_KEYS) return;
    if (m_keyDown[sc]) m_keyReleasedAccum[sc] = true;
    m_keyDown[sc] = false;
}
void KeyboardMouseInput::OnMouseButtonDown(int btn) {
    if (btn < 0 || btn >= MAX_MOUSE_BUTTONS) return;
    if (!m_mouseButtonDown[btn]) m_mouseBtnPressedAccum[btn] = true;
    m_mouseButtonDown[btn] = true;
}
void KeyboardMouseInput::OnMouseButtonUp(int btn) {
    if (btn < 0 || btn >= MAX_MOUSE_BUTTONS) return;
    if (m_mouseButtonDown[btn]) m_mouseBtnReleasedAccum[btn] = true;
    m_mouseButtonDown[btn] = false;
}
void KeyboardMouseInput::OnMouseMove(int x, int y) { m_mouseX = x; m_mouseY = y; }
void KeyboardMouseInput::OnMouseWheel(int delta)   { m_mouseWheelAccum += delta; }
void KeyboardMouseInput::OnRawMouseDelta(int dx, int dy) { m_mouseDeltaAccumX += dx; m_mouseDeltaAccumY += dy; }

bool KeyboardMouseInput::IsKeyDown(int sc)     const { return (sc >= 0 && sc < MAX_KEYS) ? m_keyDown[sc] : false; }
bool KeyboardMouseInput::IsKeyPressed(int sc)  const { return (sc >= 0 && sc < MAX_KEYS) ? m_keyPressed[sc] : false; }
bool KeyboardMouseInput::IsKeyReleased(int sc) const { return (sc >= 0 && sc < MAX_KEYS) ? m_keyReleased[sc] : false; }
int  KeyboardMouseInput::GetPressedKey()       const { return m_pressedKey; }

bool KeyboardMouseInput::IsMouseButtonDown(int b)     const { return (b>=0&&b<MAX_MOUSE_BUTTONS) ? m_mouseButtonDown[b] : false; }
bool KeyboardMouseInput::IsMouseButtonPressed(int b)  const { return (b>=0&&b<MAX_MOUSE_BUTTONS) ? m_mouseBtnPressed[b] : false; }
bool KeyboardMouseInput::IsMouseButtonReleased(int b) const { return (b>=0&&b<MAX_MOUSE_BUTTONS) ? m_mouseBtnReleased[b] : false; }

int KeyboardMouseInput::GetMouseWheel() { int v = m_mouseWheelAccum; m_mouseWheelAccum = 0; return v; }

void KeyboardMouseInput::ConsumeMouseDelta(float& dx, float& dy) {
    dx = (float)m_mouseDeltaAccumX; dy = (float)m_mouseDeltaAccumY;
    m_mouseDeltaAccumX = 0; m_mouseDeltaAccumY = 0;
}
void KeyboardMouseInput::SetMouseGrabbed(bool g)    { m_mouseGrabbed = g; SDL_SetRelativeMouseMode(g ? SDL_TRUE : SDL_FALSE); }
void KeyboardMouseInput::SetCursorHiddenForUI(bool h) { m_cursorHiddenForUI = h; SDL_ShowCursor(h ? SDL_DISABLE : SDL_ENABLE); }
void KeyboardMouseInput::SetWindowFocused(bool f)   { m_windowFocused = f; }

void KeyboardMouseInput::OnChar(wchar_t c) {
    int next = (m_charBufferTail + 1) % CHAR_BUFFER_SIZE;
    if (next != m_charBufferHead) { m_charBuffer[m_charBufferTail] = c; m_charBufferTail = next; }
}
bool KeyboardMouseInput::ConsumeChar(wchar_t& out) {
    if (m_charBufferHead == m_charBufferTail) return false;
    out = m_charBuffer[m_charBufferHead];
    m_charBufferHead = (m_charBufferHead + 1) % CHAR_BUFFER_SIZE;
    return true;
}
void KeyboardMouseInput::ClearCharBuffer() { m_charBufferHead = m_charBufferTail = 0; }

float KeyboardMouseInput::GetMoveX() const { return (m_keyDown[KEY_RIGHT]?1.f:0.f) - (m_keyDown[KEY_LEFT]?1.f:0.f); }
float KeyboardMouseInput::GetMoveY() const { return (m_keyDown[KEY_FORWARD]?1.f:0.f) - (m_keyDown[KEY_BACKWARD]?1.f:0.f); }
float KeyboardMouseInput::GetLookX(float s) const { return (float)m_mouseDeltaX * s; }
float KeyboardMouseInput::GetLookY(float s) const { return (float)m_mouseDeltaY * s; }

#endif // _LINUX64
