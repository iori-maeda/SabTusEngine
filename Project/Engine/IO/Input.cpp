#include "Input.h"
#include <format>

#include "Window/WinApp.h"
#include "Logger.h"

#pragma comment (lib, "dinput8.lib")

bool Input::Initialize(WinApp *winApp)
{
	if (winApp == nullptr) { return false; }

	winApp_ = winApp;

	if (!InitializeDirectInput()) { return false; }

	if (InitializeKeyboard()) { keyboardDevice_->Acquire(); }
	else { return false; }

	if (InitializeMouse()) { mouseDevice_->Acquire(); }
	else { return false; }

	return true;
}

void Input::Update()
{
	DIMOUSESTATE2 mouseState{};
	// キーボードの入力情報が断絶していれば取得を再開する
	if (FAILED(keyboardDevice_->GetDeviceState(sizeof(keys_), &keys_)))
	{
		keyboardDevice_->Acquire();
	}
	// マウスの入力情報が断絶していれば取得を再開する
	if(FAILED(mouseDevice_->GetDeviceState(sizeof(mouseState), &mouseState)))
	{
		mouseDevice_->Acquire();
	}

	// キーボード
	memcpy(preKeys_, keys_, kAllKey);
	keyboardDevice_->GetDeviceState(sizeof(keys_), keys_);

	// マウス
	if (isCursorVisible_)
	{
		while (ShowCursor(isCursorVisible_) < 0);
	}
	else
	{
		while (ShowCursor(isCursorVisible_) >= 0);
	}

	memcpy(mouseState_.preButtons, mouseState_.buttons, 8);
	mouseDevice_->GetDeviceState(sizeof(mouseState), &mouseState);
	// マウスの絶対座標を取得
	POINT mousePoint{};
	GetCursorPos(&mousePoint);
	ScreenToClient(winApp_->GetHWND(), &mousePoint);

	// 情報を自作構造体へ変換
	mouseState_.deltaPosition = Vector2(static_cast<float>(mouseState.lX), static_cast<float>(mouseState.lY));
	mouseState_.scPosition = Vector2(static_cast<float>(mousePoint.x), static_cast<float>(mousePoint.y));
	mouseState_.wheel = static_cast<float>(mouseState.lZ);
	memcpy(mouseState_.buttons, mouseState.rgbButtons, 8);

	if (!isMouseConroll_)
	{
		mouseState_.scPosition = Vector2(static_cast<float>(WinApp::sWindoWidth / 2), static_cast<float>(WinApp::sWindoHeight / 2));
	}
}

bool Input::InitializeDirectInput()
{
	HRESULT result = DirectInput8Create(
		winApp_->GetHInstance(),
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void **)&directInput_,
		nullptr
	);
	return result == S_OK ? true : false;
}

bool Input::InitializeKeyboard()
{
	HRESULT result = directInput_->CreateDevice(
		GUID_SysKeyboard,
		&keyboardDevice_,
		NULL
	);
	if (result != S_OK) { return false; }

	result = keyboardDevice_->SetCooperativeLevel(
		winApp_->GetHWND(),
		DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY
	);
	if (result != S_OK) { return false; }

	result = keyboardDevice_->SetDataFormat(&c_dfDIKeyboard);
	return result == S_OK ? true : false;
}

bool Input::InitializeMouse()
{
	HRESULT result = directInput_->CreateDevice(
		GUID_SysMouse,
		&mouseDevice_,
		NULL
	);
	if (result != S_OK) { return false; }

	result = mouseDevice_->SetCooperativeLevel(
		winApp_->GetHWND(),
		DISCL_FOREGROUND | DISCL_NONEXCLUSIVE
	);
	if (result != S_OK) { return false; }

	result = mouseDevice_->SetDataFormat(&c_dfDIMouse2);
	return result == S_OK ? true : false;
}
