#include "stdafx.h"

#ifdef _WINDOWS64

#include "KeyboardMouseInput.h"

KeyboardMouseInput KMInput;

KeyboardMouseInput::KeyboardMouseInput()
	: m_mouseDeltaX(0.0f)
	, m_mouseDeltaY(0.0f)
	, m_mouseDeltaXAccum(0.0f)
	, m_mouseDeltaYAccum(0.0f)
	, m_scrollDelta(0)
	, m_scrollDeltaAccum(0)
	, m_captured(false)
	, m_hWnd(NULL)
	, m_initialized(false)
{
	memset(m_keyState, 0, sizeof(m_keyState));
	memset(m_keyStatePrev, 0, sizeof(m_keyStatePrev));
	memset(m_mouseButtons, 0, sizeof(m_mouseButtons));
	memset(m_mouseButtonsPrev, 0, sizeof(m_mouseButtonsPrev));
}

KeyboardMouseInput::~KeyboardMouseInput()
{
	if (m_captured)
	{
		SetCapture(false);
	}
}

void KeyboardMouseInput::Init(HWND hWnd)
{
	m_hWnd = hWnd;
	m_initialized = true;

	// Register for raw mouse input
	RAWINPUTDEVICE rid;
	rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid.usUsage = HID_USAGE_GENERIC_MOUSE;
	rid.dwFlags = 0;
	rid.hwndTarget = hWnd;
	RegisterRawInputDevices(&rid, 1, sizeof(rid));
}

void KeyboardMouseInput::Tick()
{
	// Snapshot accumulated mouse deltas
	m_mouseDeltaX = m_mouseDeltaXAccum;
	m_mouseDeltaY = m_mouseDeltaYAccum;
	m_mouseDeltaXAccum = 0.0f;
	m_mouseDeltaYAccum = 0.0f;

	// Snapshot scroll delta
	m_scrollDelta = m_scrollDeltaAccum;
	m_scrollDeltaAccum = 0;

	// Keep cursor pinned to center while captured
	if (m_captured)
		CenterCursor();
}

void KeyboardMouseInput::EndFrame()
{
	// Advance previous state for next frame's edge detection.
	// Must be called AFTER all consumers have read IsKeyPressed/Released etc.
	memcpy(m_keyStatePrev, m_keyState, sizeof(m_keyState));
	memcpy(m_mouseButtonsPrev, m_mouseButtons, sizeof(m_mouseButtons));
}

void KeyboardMouseInput::OnKeyDown(WPARAM vk)
{
	if (vk < 256)
	{
		m_keyState[vk] = true;
	}
}

void KeyboardMouseInput::OnKeyUp(WPARAM vk)
{
	if (vk < 256)
	{
		m_keyState[vk] = false;
	}
}

void KeyboardMouseInput::OnRawMouseInput(LPARAM lParam)
{
	if (!m_captured) return;

	UINT dwSize = 0;
	GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));

	BYTE* lpb = (BYTE*)alloca(dwSize);
	if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
		return;

	RAWINPUT* raw = (RAWINPUT*)lpb;
	if (raw->header.dwType == RIM_TYPEMOUSE)
	{
		if (raw->data.mouse.usFlags == MOUSE_MOVE_RELATIVE)
		{
			m_mouseDeltaXAccum += (float)raw->data.mouse.lLastX;
			m_mouseDeltaYAccum += (float)raw->data.mouse.lLastY;
		}
	}
}

void KeyboardMouseInput::OnMouseButton(int button, bool down)
{
	if (button >= 0 && button < 3)
	{
		m_mouseButtons[button] = down;
	}
}

void KeyboardMouseInput::OnMouseWheel(int delta)
{
	m_scrollDeltaAccum += delta;
}

void KeyboardMouseInput::ClearAllState()
{
	memset(m_keyState, 0, sizeof(m_keyState));
	memset(m_mouseButtons, 0, sizeof(m_mouseButtons));
	m_mouseDeltaXAccum = 0.0f;
	m_mouseDeltaYAccum = 0.0f;
	m_scrollDeltaAccum = 0;
}

// Key queries
bool KeyboardMouseInput::IsKeyDown(int vk) const
{
	if (vk < 0 || vk >= 256) return false;
	return m_keyState[vk];
}

bool KeyboardMouseInput::IsKeyPressed(int vk) const
{
	if (vk < 0 || vk >= 256) return false;
	return m_keyState[vk] && !m_keyStatePrev[vk];
}

bool KeyboardMouseInput::IsKeyReleased(int vk) const
{
	if (vk < 0 || vk >= 256) return false;
	return !m_keyState[vk] && m_keyStatePrev[vk];
}

// Mouse button queries
bool KeyboardMouseInput::IsMouseDown(int btn) const
{
	if (btn < 0 || btn >= 3) return false;
	return m_mouseButtons[btn];
}

bool KeyboardMouseInput::IsMousePressed(int btn) const
{
	if (btn < 0 || btn >= 3) return false;
	return m_mouseButtons[btn] && !m_mouseButtonsPrev[btn];
}

bool KeyboardMouseInput::IsMouseReleased(int btn) const
{
	if (btn < 0 || btn >= 3) return false;
	return !m_mouseButtons[btn] && m_mouseButtonsPrev[btn];
}

// Delta queries
float KeyboardMouseInput::GetMouseDeltaX() const { return m_mouseDeltaX; }
float KeyboardMouseInput::GetMouseDeltaY() const { return m_mouseDeltaY; }
int KeyboardMouseInput::GetScrollDelta() const { return m_scrollDelta; }

// Mouse capture
void KeyboardMouseInput::SetCapture(bool capture)
{
	if (capture == m_captured) return;
	m_captured = capture;

	if (capture)
	{
		ShowCursor(FALSE);
		RECT rect;
		GetClientRect(m_hWnd, &rect);
		POINT topLeft = { rect.left, rect.top };
		POINT bottomRight = { rect.right, rect.bottom };
		ClientToScreen(m_hWnd, &topLeft);
		ClientToScreen(m_hWnd, &bottomRight);
		RECT screenRect = { topLeft.x, topLeft.y, bottomRight.x, bottomRight.y };
		ClipCursor(&screenRect);
		CenterCursor();

		// Flush accumulated deltas so the snap-to-center doesn't cause a jump
		m_mouseDeltaXAccum = 0.0f;
		m_mouseDeltaYAccum = 0.0f;
	}
	else
	{
		ShowCursor(TRUE);
		ClipCursor(NULL);
	}
}

bool KeyboardMouseInput::IsCaptured() const { return m_captured; }

void KeyboardMouseInput::CenterCursor()
{
	RECT rect;
	GetClientRect(m_hWnd, &rect);
	POINT center = { (rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2 };
	ClientToScreen(m_hWnd, &center);
	SetCursorPos(center.x, center.y);
}

#endif // _WINDOWS64
