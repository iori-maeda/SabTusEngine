#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <array>

#include "ComPtr.h"
#include "Math/Vector2.h"

typedef uint32_t KeyCode;

class WinApp;

class Input
{
public:

	struct MouseState
	{
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

	Vector2 GetMousePosition()const { return mouseState_.scPosition; }

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

	MouseState mouseState_{};
};