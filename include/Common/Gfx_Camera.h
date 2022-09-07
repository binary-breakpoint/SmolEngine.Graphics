#pragma once
#include "Common/Gfx_Events.h"

namespace SmolEngine
{
	class Gfx_Camera;

	struct Gfx_ViewProjection
	{
		Gfx_ViewProjection() = default;
		Gfx_ViewProjection(Gfx_Camera* cam);

		void Update(Gfx_Camera* cam);

		glm::mat4 myView = glm::mat4(1.0f);
		glm::mat4 myProjection = glm::mat4(1.0f);
		glm::mat4 myViewInverse = glm::mat4(1.0f);
		glm::mat4 myProjectionInverse = glm::mat4(1.0f);
		glm::mat4 myViewProj = glm::mat4(1.0f);
		glm::mat4 myViewProjInverse = glm::mat4(1.0f);
		glm::mat4 myProjectionNJ = glm::mat4(1.0f); // non jittered!
		glm::vec2 myPixelOffset = glm::vec2(0);
	};

	struct CameraCreateDesc
	{
		float myFOV = 60.0f;
		float myNearClip = 0.01f;
		float myFarClip = 1000.0f;
		float mySpeed = 0.5f;
		glm::vec3 myPos = glm::vec3(0, 0, 0);
		glm::vec3 myFocalPos = glm::vec3(0, 0, 0);
		uint32_t myWidth = 720;
		uint32_t myHeight = 480;
	};

	class Gfx_Camera
	{
	public:
		Gfx_Camera() = default;
		virtual ~Gfx_Camera() = default;

		virtual void OnUpdate(float deltaTime) = 0;
		virtual void OnResize(uint32_t width, uint32_t height) = 0;
		virtual void OnEvent(Gfx_Event& e) {};


		virtual const glm::mat4  GetViewProjection() const = 0;
		virtual const glm::mat4& GetProjection() const = 0;
		virtual const glm::mat4& GetViewMatrix() const = 0;
		virtual const glm::vec3& GetPosition() const = 0;

		virtual float GetNearClip() const = 0;
		virtual float GetFarClip() const = 0;

		Gfx_ViewProjection* GetSceneViewProjection() { return &m_SceneViewProjection; }

		glm::mat4 m_ViewMatrixOpenGL, m_ProjectionMatrixOpenGL;

	protected:
		Gfx_ViewProjection m_SceneViewProjection;
	};
}