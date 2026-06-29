#include "Camera.h"
#include "Window/WinApp.h"
#include "ImGuiManager.h"
#include "../IO/Input.h"
#include "../WIndow/WinApp.h"

void Camera::Initialize()
{
	transform_.scale = Vector3(1.0f, 1.0f, 1.0f);
	transform_.rotate = Vector3();
	transform_.translate = Vector3(0.0f, 0.0f, -10.0f);

	worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	viewMatrix_ = MakeInVerse(worldMatrix_);
	projectionMatrix_ = MakePerspectiveFovMatrix(fovAngleY_, static_cast<float>(WinApp::sWindoWidth) / static_cast<float>(WinApp::sWindoHeight), nearZ_, farZ_);
}

void Camera::Update()
{
	worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	viewMatrix_ = MakeInVerse(worldMatrix_);

	forward_ = Normalize(ConvertToTransform(Vector3(0.0f, 0.0f, 1.0f), MakeRotationMatrix3x3(worldMatrix_)));
	right_ = Normalize(ConvertToTransform(Vector3(1.0f, 0.0f, 0.0f), MakeRotationMatrix3x3(worldMatrix_)));
	top_ = Normalize(ConvertToTransform(Vector3(0.0f, 1.0f, 0.0f), MakeRotationMatrix3x3(worldMatrix_)));
	switch (projMode_)
	{
	case Camera::ProjectionMode::Perspective:
		projectionMatrix_ = MakePerspectiveFovMatrix(fovAngleY_, static_cast<float>(WinApp::sWindoWidth) / static_cast<float>(WinApp::sWindoHeight), nearZ_, farZ_);
		break;
	case Camera::ProjectionMode::Orthographic:
		projectionMatrix_ = MakeOrthoGraphicsMatrix(0.0f, 0.0f, static_cast<float>(WinApp::sWindoWidth), static_cast<float>(WinApp::sWindoHeight), nearZ_, farZ_);
		break;
	}
}

void Camera::DebugWindow()
{
#ifdef USE_IMGUI
	ImGui::Begin("Camera");
	ImGui::DragFloat3("position", &transform_.translate.x, 0.01f);
	ImGui::DragFloat3("rotation", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("scale", &transform_.scale.x, 0.01f);
	ImGui::DragFloat("fovAngleY", &fovAngleY_, 0.01f, 0.1f, 3.14f);
	ImGui::DragFloat("nearZ", &nearZ_, 0.01f, 0.01f, 100.0f);
	ImGui::DragFloat("farZ", &farZ_, 0.1f, 10.0f, 1000.0f);
	static const char *projStr[] = { "Perspective", "Orthographic" };
	static ProjectionMode projMode = projMode_;
	if (ImGui::BeginCombo("ProjectionMode", projStr[static_cast<int>(projMode)]))
	{
		for (int i = 0; i < static_cast<int>(ProjectionMode::ModeCount); i++)
		{
			bool isSelected = (projMode == static_cast<ProjectionMode>(i));
			if (ImGui::Selectable(projStr[i], isSelected))
			{
				projMode = static_cast<ProjectionMode>(i);
				SetProjectionMode(projMode);
			}
			if (isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	ImGui::End();
#endif 
}

void Camera::DebugCameraMode()
{
#ifdef _DEBUG
	const float kCameraMoveSpeed = 0.05f;

	if (input_->PushKey(DIK_W))
	{
		transform_.translate.x += forward_.x * kCameraMoveSpeed;
		transform_.translate.z += forward_.z * kCameraMoveSpeed;
	}
	if (input_->PushKey(DIK_S))
	{
		transform_.translate.x += forward_.x * -kCameraMoveSpeed;
		transform_.translate.z += forward_.z * -kCameraMoveSpeed;
	}
	if (input_->PushKey(DIK_A))
	{
		transform_.translate.x += right_.x * -kCameraMoveSpeed;
		transform_.translate.z += right_.z * -kCameraMoveSpeed;
	}
	if (input_->PushKey(DIK_D))
	{
		transform_.translate.x += right_.x * kCameraMoveSpeed;
		transform_.translate.z += right_.z * kCameraMoveSpeed;
	}

	if (!input_->OnDebugWindow())
	{
		Vector2 mousePosition = input_->GetMousePosition();
		if (winRect_.left <= mousePosition.x && winRect_.right >= mousePosition.x
			&& winRect_.top <= mousePosition.y && winRect_.bottom >= mousePosition.y)
		{
			transform_.translate += forward_ * (input_->GetMouseWheel() / 100.0f);
		}
	}

	input_->SetCursorVisible(true);
	input_->SetMouseControll(true);
	if (!input_->OnDebugWindow() || WinApp::sIsCursorOverTitleBar)
	{
		if (input_->PushMouseButton(MouseButton::LEFT))
		{
			input_->SetCursorVisible(false);
			input_->SetMouseControll(false);
			Vector2 dir = input_->GetDeltaMousePosition();
			transform_.rotate.x += dir.y * 0.005f;
			transform_.rotate.y += dir.x * 0.005f;
		}
		if (input_->PushMouseButton(MouseButton::WHEEL))
		{
			input_->SetCursorVisible(false);
			input_->SetMouseControll(false);
			Vector2 dir = input_->GetDeltaMousePosition();
			transform_.translate += right_ * -dir.x * 0.005f;
			transform_.translate += top_ * dir.y * 0.005f;
		}
	}
#endif // DEBUG
}
