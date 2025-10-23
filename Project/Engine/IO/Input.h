#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <array>

#include "ComPtr.h"
#include "Math/Vector2.h"

typedef uint32_t KeyCode;

class WinApp;

enum class MouseButton :int8_t
{
	LEFT,
	RIGHT,
	WHEEL,
	OTHER4,
	OTHER5,
	OTHER6,
	OTHER7,
	OTHER8
};

class Input
{
public:

	struct MouseState
	{
		Vector2 deltaPosition{};
		Vector2 scPosition{};
		float wheel = 0.0f;
		BYTE buttons[8]{};
		BYTE preButtons[8]{};
	};


public:
	Input() = default;
	~Input() = default;

	bool Initialize(WinApp *winApp);
	void Update();

	bool PushKey(KeyCode keyCode) { return keys_[keyCode]; }
	bool TriggerKey(KeyCode keyCode) { return keys_[keyCode] && !preKeys_[keyCode]; }
	bool ReleaseKey(KeyCode keyCode) { return !keys_[keyCode] && preKeys_[keyCode]; }

	Vector2 GetDeltaMousePosition()const { return mouseState_.deltaPosition; }
	Vector2 GetMousePosition()const { return mouseState_.scPosition; }
	float GetMouseWheel() const { return mouseState_.wheel; }
	bool PushMouseButton(MouseButton button) const { return mouseState_.buttons[static_cast<int8_t>(button)]; }
	bool TriggerMouseButton(MouseButton button) const { return mouseState_.buttons[static_cast<int8_t>(button)] && !mouseState_.preButtons[static_cast<int8_t>(button)]; }
	bool ReleaseMouseButton(MouseButton button) const { return !mouseState_.buttons[static_cast<int8_t>(button)] && mouseState_.preButtons[static_cast<int8_t>(button)]; }

	void SetCursorVisible(bool flag) { isCursorVisible_ = flag; }
	void SetMouseControll(bool flag) { isMouseConroll_ = flag; }

private:

	bool InitializeDirectInput();
	bool InitializeKeyboard();
	bool InitializeMouse();

private:
	WinApp *winApp_ = nullptr;

	ComPtr<IDirectInput8> directInput_ = nullptr;
	ComPtr<IDirectInputDevice8> keyboardDevice_ = nullptr;
	ComPtr<IDirectInputDevice8> mouseDevice_ = nullptr;

	static const int kAllKey = 256;
	BYTE keys_[kAllKey]{};
	BYTE preKeys_[kAllKey]{};

	bool isCursorVisible_ = true;
	bool isMouseConroll_ = true;
	MouseState mouseState_{};
};