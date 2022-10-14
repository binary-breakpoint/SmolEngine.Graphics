#pragma once

// MSVC ENABLE/DISABLE WARNING DEFINITION
#	define VKBP_DISABLE_WARNINGS() \
		__pragma(warning(push, 0))

#	define VKBP_ENABLE_WARNINGS() \
		__pragma(warning(pop))

VKBP_DISABLE_WARNINGS()
#include <glslang/Public/ShaderLang.h>
VKBP_DISABLE_WARNINGS()

#include <map>
#include <vector>

namespace SmolEngine
{
	class Gfx_ShaderIncluder final : public glslang::TShader::Includer
	{
	public:
		using IncResult = glslang::TShader::Includer::IncludeResult;

		Gfx_ShaderIncluder();

		// For the "system" or <>-style includes; search the "system" paths.
		virtual IncResult* includeSystem(const char* /*headerName*/,
			const char* /*includerName*/,
			size_t /*inclusionDepth*/) override;

		virtual IncResult* includeLocal(const char* /*headerName*/,
			const char* /*includerName*/,
			size_t /*inclusionDepth*/) override;

		virtual void releaseInclude(IncResult* incl) override;

		static void AddIncludeDir(const std::string& dir);
		static void Clear();

		static Gfx_ShaderIncluder* GetSingleton() { return s_Instance; }
		static std::vector<std::string>& GetIncludeDirs() { return s_Instance->m_IncludeDirs; }

	private:
		inline static Gfx_ShaderIncluder* s_Instance = nullptr;
		std::map<std::string, IncResult*>  m_Includes;
		std::map<std::string, std::string> m_Sources;
		std::vector<std::string> m_IncludeDirs;
	};
}