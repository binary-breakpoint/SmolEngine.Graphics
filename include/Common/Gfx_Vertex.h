#pragma once
#include "Common/Gfx_BufferLayout.h"

#include <glm/glm.hpp>

namespace SmolEngine
{
	struct PBRVertex // TODO: compress
	{
		glm::vec3 Pos = glm::vec3(0.0f);
		glm::vec3 Normals = glm::vec3(0.0f);
		glm::vec4 Tangent = glm::vec4(0.0f);
		glm::vec2 UVs = glm::vec2(0.0f);
		glm::ivec4 jointIndices = glm::ivec4(0);
		glm::vec4 jointWeight = glm::vec4(0.0f);
	};
}