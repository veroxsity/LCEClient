#pragma once

#ifdef _WINDOWS64

#include <windows.h>

// HID usage page and usage for raw input registration
#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC ((USHORT)0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE ((USHORT)0x02)
#endif

class KeyboardMouseInput
{
public:
	KeyboardMouseInput();
	~KeyboardMouseInput();

	void Init(HWND hWnd);
	void Tick();
	void EndFrame();

	// Called from WndProc
	void OnKeyDown(WPARAM vk);
	void OnKeyUp(WPARAM vk);
	void OnRawMouseInput(LPARAM lParam);
	void OnMouseButton(int button, bool down);
	void OnMouseWheel(int delta);
	void ClearAllState();

	// Key state queries (call after Tick)
	bool IsKeyDown(int vk) const;
	bool IsKeyPressed(int vk) const;
	bool IsKeyReleased(int vk) const;

	// Mouse button queries: 0=left, 1=right, 2=middle
	bool IsMouseDown(int btn) const;
	bool IsMousePressed(int btn) const;
	bool IsMouseReleased(int btn) const;

	// Mouse deltas (consumed each Tick)
	float GetMouseDeltaX() const;
	float GetMouseDeltaY() const;
	int GetScrollDelta() const;

	// Mouse capture for FPS look
	void SetCapture(bool capture);
	bool IsCaptured() const;

private:
	void CenterCursor();

	bool m_keyState[256];
	bool m_keyStatePrev[256];

	bool m_mouseButtons[3];
	bool m_mouseButtonsPrev[3];

	float m_mouseDeltaX;
	float m_mouseDeltaY;
	float m_mouseDeltaXAccum;
	float m_mouseDeltaYAccum;

	int m_scrollDelta;
	int m_scrollDeltaAccum;

	bool m_captured;
	HWND m_hWnd;
	bool m_initialized;
};

extern KeyboardMouseInput KMInput;

#endif // _WINDOWS64
