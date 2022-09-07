#include "Gfx_Precompiled.h"
#include "Common/Gfx_Camera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_major_storage.hpp>

namespace SmolEngine
{

	Gfx_ViewProjection::Gfx_ViewProjection(Gfx_Camera* cam)
	{
		Update(cam);
	}

	void Gfx_ViewProjection::Update(Gfx_Camera* cam)
	{
		myView = cam->GetViewMatrix();
		myViewInverse = glm::inverse(myView);

		myProjection = cam->GetProjection();
		myProjectionInverse = glm::inverse(myProjection);
		myProjectionNJ = myProjection;

		myViewProj = myProjection * myViewProj;
		myViewProjInverse = myProjectionInverse * myViewInverse;

		myPixelOffset = glm::vec2(0); //fix
	}
}