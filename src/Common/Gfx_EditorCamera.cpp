#include "Gfx_Precompiled.h"
#include "Common/Gfx_EditorCamera.h"
#include "Common/Gfx_Framebuffer.h"
#include "Common/gfx_Input.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <glm/gtx/matrix_major_storage.hpp>

#include <iostream>
#include <cmath>

#define M_PI 3.14159f

namespace Dia
{
	Gfx_EditorCamera::Gfx_EditorCamera(EditorCameraCreateDesc* ci)
	{
		m_ViewportWidth = ci->myWidth;
		m_ViewportHeight = ci->myHeight;
		m_FOV = ci->myFOV;
		m_NearClip = ci->myNearClip;
		m_FarClip = ci->myFarClip;
		m_Speed = ci->mySpeed;

		m_ProjectionMatrix = glm::perspectiveFov(glm::radians(m_FOV),
			float(m_ViewportWidth), float(m_ViewportHeight), m_NearClip, m_FarClip);

		m_FocalPoint = ci->myFocalPos;
		m_Position = ci->myPos;

		m_Distance = glm::distance(m_Position, m_FocalPoint);

		m_Yaw = ci->myYaw;
		m_Pitch = ci->myPitch;

		const glm::quat orientation = GetOrientation();
		m_WorldRotation = glm::eulerAngles(orientation) * (180.0f / (float)M_PI);
		m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
		//m_ViewMatrix = glm::inverse(m_ViewMatrix);

		m_SceneViewProjection.Update(this);
	}

	static void DisableMouse()
	{
		//Gfx_Input::SetCursorMode(CursorMode::Locked);
		//UI::SetMouseEnabled(false);
	}
	static void EnableMouse()
	{
		//Gfx_Input::SetCursorMode(CursorMode::Normal);
		//UI::SetMouseEnabled(true);
	}

	void Gfx_EditorCamera::OnResize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		const float aspectRatio = float(m_ViewportWidth) / float(m_ViewportHeight);
		float yScale = 1.0f / tanf(0.5f * glm::radians(m_FOV));
		float xScale = yScale / aspectRatio;

		m_ProjectionMatrix = glm::perspectiveFov(glm::radians(m_FOV), 
			float(m_ViewportWidth), float(m_ViewportHeight), m_NearClip, m_FarClip);
		m_SceneViewProjection.Update(this);
	}

	void Gfx_EditorCamera::OnUpdate(float ts)
	{
		const glm::vec2& mouse{ Gfx_Input::GetMouseX(), Gfx_Input::GetMouseY() };
		const glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.002f;

		if (m_IsActive)
		{
			if (Gfx_Input::IsMouseButtonPressed(MouseCode::ButtonRight) && !Gfx_Input::IsKeyPressed(KeyCode::LeftAlt))
			{
				m_CameraMode = CameraMode::FLYCAM;
				DisableMouse();
				const float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;

				if (Gfx_Input::IsKeyPressed(KeyCode::Q))
					m_PositionDelta -= ts * m_Speed * glm::vec3{ 0.f, yawSign, 0.f };
				if (Gfx_Input::IsKeyPressed(KeyCode::E))
					m_PositionDelta += ts * m_Speed * glm::vec3{ 0.f, yawSign, 0.f };
				if (Gfx_Input::IsKeyPressed(KeyCode::S))
					m_PositionDelta -= ts * m_Speed * m_WorldRotation;
				if (Gfx_Input::IsKeyPressed(KeyCode::W))
					m_PositionDelta += ts * m_Speed * m_WorldRotation;
				if (Gfx_Input::IsKeyPressed(KeyCode::A))
					m_PositionDelta -= ts * m_Speed * m_RightDirection;
				if (Gfx_Input::IsKeyPressed(KeyCode::D))
					m_PositionDelta += ts * m_Speed * m_RightDirection;

				constexpr float maxRate{ 0.12f };
				m_YawDelta += glm::clamp(yawSign * delta.x * RotationSpeed(), -maxRate, maxRate);
				m_PitchDelta += glm::clamp(delta.y * RotationSpeed(), -maxRate, maxRate);

				m_RightDirection = glm::cross(m_WorldRotation, glm::vec3{ 0.f, 1.0f, 0.f });

				m_WorldRotation = glm::rotate(glm::normalize(glm::cross(glm::angleAxis(-m_PitchDelta, m_RightDirection),
					glm::angleAxis(-m_YawDelta, glm::vec3{ 0.f, yawSign, 0.f }))), m_WorldRotation);
			}
			else if (Gfx_Input::IsKeyPressed(KeyCode::LeftAlt))
			{
				m_CameraMode = CameraMode::ARCBALL;

				if (Gfx_Input::IsMouseButtonPressed(MouseCode::ButtonMiddle))
				{
					DisableMouse();
					MousePan(delta);
				}
				else if (Gfx_Input::IsMouseButtonPressed(MouseCode::ButtonLeft))
				{
					DisableMouse();
					MouseRotate(delta);
				}
				else if (Gfx_Input::IsMouseButtonPressed(MouseCode::ButtonRight))
				{
					DisableMouse();
					MouseZoom(delta.x + delta.y);
				}
				else
					EnableMouse();
			}
			else
			{
				EnableMouse();
			}
		}
		m_InitialMousePosition = mouse;

		m_Position += m_PositionDelta;
		m_Yaw += m_YawDelta;
		m_Pitch += m_PitchDelta;

		if (m_CameraMode == CameraMode::ARCBALL)
			m_Position = CalculatePosition();

		UpdateCameraView();
	}

	void Gfx_EditorCamera::UpdateCameraView()
	{
		const float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;

		// Extra step to handle the problem when the camera direction is the same as the up vector
		const float cosAngle = glm::dot(GetForwardDirection(), GetUpDirection());
		if (cosAngle * yawSign > 0.99f)
			m_PitchDelta = 0.f;

		const glm::vec3 lookAt = m_Position + GetForwardDirection();
		m_WorldRotation = glm::normalize(m_FocalPoint - m_Position);
		m_FocalPoint = m_Position + GetForwardDirection() * m_Distance;
		m_Distance = glm::distance(m_Position, m_FocalPoint);
		m_ViewMatrix = glm::lookAt(m_Position, lookAt, glm::vec3{ 0.f, yawSign, 0.f });
		//damping for smooth camera
		m_YawDelta *= 0.6f;
		m_PitchDelta *= 0.6f;
		m_PositionDelta *= 0.8f;

		m_SceneViewProjection.Update(this);
	}

	void Gfx_EditorCamera::Focus(const glm::vec3& focusPoint)
	{
		m_FocalPoint = focusPoint;
		if (m_Distance > m_MinFocusDistance)
		{
			const float distance = m_Distance - m_MinFocusDistance;
			MouseZoom(distance / ZoomSpeed());
			m_CameraMode = CameraMode::ARCBALL;
		}
		m_Position = m_FocalPoint - GetForwardDirection() * m_Distance;
		UpdateCameraView();
	}

	std::pair<float, float> Gfx_EditorCamera::PanSpeed() const
	{
		const float x = std::min(float(m_ViewportWidth) / 1000.0f, 24.0f); // max = 2.4f
		const float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		const float y = std::min(float(m_ViewportHeight) / 1000.0f, 24.0f); // max = 2.4f
		const float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { m_PanSpeed * xFactor, m_PanSpeed * yFactor };
	}

	float Gfx_EditorCamera::RotationSpeed() const
	{
		return m_RotationSpeed;
	}

	float Gfx_EditorCamera::ZoomSpeed() const
	{
		float distance = m_Distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
	}

	void Gfx_EditorCamera::OnEvent(Gfx_Event& event)
	{
		if (event.IsType(Gfx_Event::Type::MOUSE_SCROLL))
		{
			MouseScrollEvent* scroll = event.Cast<MouseScrollEvent>();
			OnMouseScroll(*scroll);
		}

		if (event.IsType(Gfx_Event::Type::KEY_RELEASE))
		{
			KeyEvent* key = event.Cast<KeyEvent>();
			OnKeyReleased(*key);
		}

		if (event.IsType(Gfx_Event::Type::KEY_PRESS))
		{
			KeyEvent* key = event.Cast<KeyEvent>();
			OnKeyPressed(*key);
		}
	}

	bool Gfx_EditorCamera::OnMouseScroll(MouseScrollEvent& e)
	{
		if (m_IsActive)
		{
			if (Gfx_Input::IsMouseButtonPressed(MouseCode::ButtonRight))
			{
				e.GetYoffset() > 0 ? m_Speed += 0.3f * m_Speed : m_Speed -= 0.3f * m_Speed;
				m_Speed = std::clamp(m_Speed, 0.0005f, 2.f);
			}
			else
			{
				MouseZoom(e.GetYoffset() * 0.1f);
				UpdateCameraView();
			}
		}

		return false;
	}

	bool Gfx_EditorCamera::OnKeyPressed(KeyEvent& e)
	{
		if (m_LastSpeed == 0.0f)
		{
			if (e.m_Key == (uint32_t)KeyCode::LeftShift)
			{
				m_LastSpeed = m_Speed;
				m_Speed *= 2.0f - glm::log(m_Speed);
			}
			if (e.m_Key == (uint32_t)KeyCode::LeftControl)
			{
				m_LastSpeed = m_Speed;
				m_Speed /= 2.0f - glm::log(m_Speed);
			}

			m_Speed = glm::clamp(m_Speed, 0.0005f, 2.0f);
		}
		return true;
	}

	bool Gfx_EditorCamera::OnKeyReleased(KeyEvent& e)
	{
		if (e.m_Key == (uint32_t)KeyCode::LeftShift || e.m_Key == (uint32_t)KeyCode::LeftControl)
		{
			if (m_LastSpeed != 0.0f)
			{
				m_Speed = m_LastSpeed;
				m_LastSpeed = 0.0f;
			}
			m_Speed = glm::clamp(m_Speed, 0.0005f, 2.0f);
		}
		return true;
	}

	void Gfx_EditorCamera::MousePan(const glm::vec2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
	}

	void Gfx_EditorCamera::MouseRotate(const glm::vec2& delta)
	{
		const float yawSign = GetUpDirection().y < 0.0f ? -1.0f : 1.0f;
		m_YawDelta += yawSign * delta.x * RotationSpeed();
		m_PitchDelta += delta.y * RotationSpeed();
	}

	void Gfx_EditorCamera::MouseZoom(float delta)
	{
		m_Distance -= delta * ZoomSpeed();
		m_Position = m_FocalPoint - GetForwardDirection() * m_Distance;
		const glm::vec3 forwardDir = GetForwardDirection();
		if (m_Distance < 1.0f)
		{
			m_FocalPoint += forwardDir;
			m_Distance = 1.0f;
		}
		m_PositionDelta += delta * ZoomSpeed() * forwardDir;
	}

	glm::vec3 Gfx_EditorCamera::GetUpDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 Gfx_EditorCamera::GetRightDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(1.f, 0.f, 0.f));
	}

	glm::vec3 Gfx_EditorCamera::GetForwardDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::vec3 Gfx_EditorCamera::CalculatePosition() const
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance + m_PositionDelta;
	}

	glm::quat Gfx_EditorCamera::GetOrientation() const
	{
		return glm::quat(glm::vec3(-m_Pitch - m_PitchDelta, -m_Yaw - m_YawDelta, 0.0f));
	}
}