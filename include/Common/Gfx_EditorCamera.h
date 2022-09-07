#pragma once
#include "Common/Gfx_Camera.h"
#include "Common/Gfx_Events.h"

namespace SmolEngine
{
	class Gfx_Framebuffer;

	enum class CameraMode
	{
		NONE, FLYCAM, ARCBALL
	};

	struct EditorCameraCreateDesc : public CameraCreateDesc
	{
		float myYaw = 0.0f;
		float myPitch = 0.0f;
	};

	class Gfx_EditorCamera final: public Gfx_Camera
	{
	public:
		Gfx_EditorCamera() = default;
		Gfx_EditorCamera(EditorCameraCreateDesc* desc);

		void Focus(const glm::vec3& focusPoint);
		void OnUpdate(float ts);
		void OnEvent(Gfx_Event& e);
		void OnResize(uint32_t width, uint32_t height) override;

		bool IsActive() const { return m_IsActive; }
		void SetActive(bool active) { m_IsActive = active; }

		inline float GetDistance() const { return m_Distance; }
		inline void SetDistance(float distance) { m_Distance = distance; }

		const glm::vec3& GetFocalPoint() const { return m_FocalPoint; }

		virtual const glm::mat4  GetViewProjection() const override { return m_ProjectionMatrix * m_ViewMatrix; }
		virtual const glm::mat4& GetProjection() const override { return m_ProjectionMatrix; }
		virtual const glm::mat4& GetViewMatrix() const override { return m_ViewMatrix; }
		virtual const glm::vec3& GetPosition() const override { return m_Position; }

		const glm::mat4& GetProjectionOpenGL() const { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrixOpenGL() const { return m_ViewMatrix; }

		virtual float GetNearClip() const override { return m_NearClip; }
		virtual float GetFarClip() const override { return m_FarClip; }

		glm::vec3 GetUpDirection() const;
		glm::vec3 GetRightDirection() const;
		glm::vec3 GetForwardDirection() const;
		glm::quat GetOrientation() const;

		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }
		float& GetCameraSpeed() { return m_Speed; }
		float GetCameraSpeed() const { return m_Speed; }
	private:
		void UpdateCameraView();

		bool OnMouseScroll(MouseScrollEvent& e);
		bool OnKeyPressed(KeyEvent& e);
		bool OnKeyReleased(KeyEvent& e);

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);

		glm::vec3 CalculatePosition() const;

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;
	private:
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
		glm::vec3 m_Position, m_WorldRotation, m_FocalPoint;

		float m_Exposure = 0.8f;
		float m_FOV = 60.0f;
		float m_RotationSpeed = 0.6f;
		float m_PanSpeed = 2.0f;
		float m_NearClip = 0.01f;
		float m_FarClip = 1000.0f;
		bool m_IsActive = true;
		bool m_Panning, m_Rotating;
		glm::vec2 m_InitialMousePosition{};
		glm::vec3 m_InitialFocalPoint, m_InitialRotation;

		float m_Distance;
		float m_Speed{ 0.002f };
		float m_LastSpeed = 0.f;

		float m_Pitch, m_Yaw;
		float m_PitchDelta{}, m_YawDelta{};
		glm::vec3 m_PositionDelta{};
		glm::vec3 m_RightDirection{};

		CameraMode m_CameraMode{ CameraMode::FLYCAM };

		float m_MinFocusDistance = 100.0f;

		uint32_t m_ViewportWidth = 1280, m_ViewportHeight = 720;
		friend class EditorLayer;
	};
}