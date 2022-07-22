#pragma once
#include <string>
#include <unordered_map>

namespace Dia
{
	enum class OzzPath
	{
		Model,
		Skeleton,
		Animation
	};

	class Gfx_AnimImporter
	{
	public:
		static bool ImportGltf(const std::string& filePath, const std::string& exePath = "");
	};
}