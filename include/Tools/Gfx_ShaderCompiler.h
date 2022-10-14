#pragma once
#include "Common/Gfx_Shader.h"

namespace SmolEngine
{
	struct ShaderCompileDesc
	{
		std::string myFilePath = "";
		std::string myFilePathOut = "";
		ShaderStage myStage = ShaderStage::Fragment;
		std::map<std::string, bool> myDefines;
		bool myOptimize = true;
		bool myDebug = false;
	};

	class Gfx_ShaderCompiler
	{
	public:
		static void CompileSPIRV(const ShaderCompileDesc& desc, std::vector<uint32_t>& out_binaries);
		static void LoadSPIRV(const std::string& path, std::vector<uint32_t>& out_binaries);

		static void SpirvToGlsl(const ShaderCompileDesc& desc);
		static void SpirvToHlsl(const ShaderCompileDesc& desc);

		static void GlslToSpirv(const ShaderCompileDesc& desc, std::vector<uint32_t>& out_binaries);
		static void HlslToSpirv(const ShaderCompileDesc& desc, std::vector<uint32_t>& out_binaries);
	};
}