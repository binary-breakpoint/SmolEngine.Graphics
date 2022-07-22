#pragma once
#include "Common/Gfx_Flags.h"

#include <optional>
#include <string>
#include <glm/glm.hpp>

namespace Dia
{
	enum class CachedPathType
	{
		Shader,
		Pipeline
	};

	class Gfx_Helpers
	{
	public:
		static uint32_t GetFormatSize(Format format);

		static glm::mat4 ComposeTransform(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale);
		static glm::mat3x4 ComposeTransform3x4(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale);
		static glm::mat4 ComposeTransform2D(const glm::vec2& translation, const glm::vec2& rotation, const glm::vec2& scale);
		static bool DecomposeTransform(const glm::mat4& transform, glm::vec3& out_translation, glm::vec3& out_rotation, glm::vec3& out_scale);
		static std::optional<std::string> OpenFile(const char* filter);
		static std::optional<std::string> SaveFile(const char* filter, const std::string& initialName = "");
		static glm::vec3 ScreenToWorld(const glm::vec2& mousePos, float width, float height, const glm::mat4& viewProj);
		static glm::vec3 CastRay(const glm::vec3& startPos, const glm::vec2& mousePos, float width, float height, float distance, const glm::mat4& viewProj);
		static glm::vec3 CastRay(const glm::vec3& startPos, float distance, const glm::mat4& viewProj);
		static bool IsPathValid(const std::string& path);
		static std::string GetCachedPath(const std::string& filePath, CachedPathType type);
	};
}