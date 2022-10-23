#include "Gfx_Precompiled.h"
#include "Tools/Gfx_ShaderCompiler.h"
#include "Common/Gfx_Helpers.h"
#include "Tools/Gfx_ShaderIncluder.h"
#include "Backend/Gfx_VulkanHelpers.h"

VKBP_DISABLE_WARNINGS()
#include <glslang/Include/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>
VKBP_DISABLE_WARNINGS()

#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>
#include <spirv_cross/spirv_hlsl.hpp>

namespace SmolEngine
{
	class TPreamble {
	public:
		TPreamble() { }

		bool isSet() const { return text.size() > 0; }
		const char* get() const { return text.c_str(); }

		// #define...
		void addDef(std::string def)
		{
			text.append("#define ");
			fixLine(def);

			Processes.push_back("define-macro ");
			Processes.back().append(def);

			// The first "=" needs to turn into a space
			const size_t equal = def.find_first_of('=');
			if (equal != def.npos)
				def[equal] = ' ';

			text.append(def);
			text.append("\n");
		}

		// #undef...
		void addUndef(std::string undef)
		{
			text.append("#undef ");
			fixLine(undef);

			Processes.push_back("undef-macro ");
			Processes.back().append(undef);

			text.append(undef);
			text.append("\n");
		}

	protected:
		void fixLine(std::string& line)
		{
			// Can't go past a newline in the line
			const size_t end = line.find_first_of('\n');
			if (end != line.npos)
				line = line.substr(0, end);
		}

		std::vector<std::string> Processes;
		std::string text;  // contents of preamble
	};

	EShLanguage locGetShaderType(ShaderStage type)
	{
		switch (type)
		{
		case ShaderStage::Vertex:          return EShLangVertex;
		case ShaderStage::Fragment:        return EShLangFragment;
		case ShaderStage::Geometry:        return EShLangGeometry;
		case ShaderStage::Compute:         return EShLangCompute;
		case ShaderStage::RayGen:          return EShLangRayGen;

		case ShaderStage::RayAnyHit_0:     return EShLangAnyHit;
		case ShaderStage::RayAnyHit_1:     return EShLangAnyHit;
		case ShaderStage::RayAnyHit_2:     return EShLangAnyHit;
		case ShaderStage::RayAnyHit_3:     return EShLangAnyHit;

		case ShaderStage::RayCloseHit_0:   return EShLangClosestHit;
		case ShaderStage::RayCloseHit_1:   return EShLangClosestHit;
		case ShaderStage::RayCloseHit_2:   return EShLangClosestHit;
		case ShaderStage::RayCloseHit_3:   return EShLangClosestHit;

		case ShaderStage::RayMiss_0:       return EShLangMiss;
		case ShaderStage::RayMiss_1:       return EShLangMiss;
		case ShaderStage::RayMiss_2:       return EShLangMiss;
		case ShaderStage::RayMiss_3:       return EShLangMiss;

		case ShaderStage::Callable_0:      return EShLangCallable;
		case ShaderStage::Callable_1:      return EShLangCallable;
		case ShaderStage::Callable_2:      return EShLangCallable;
		case ShaderStage::Callable_3:      return EShLangCallable;
		}

		return EShLangVertex;
	}

	// TODO: error handling
	void Gfx_ShaderCompiler::CompileSPIRV(const ShaderCompileDesc& desc, std::vector<uint32_t>& out_binaries)
	{
		const bool isHLSL = desc.myFilePath.find("hlsl") != std::string::npos;
		if (isHLSL)
			HlslToSpirv(desc, out_binaries);
		else
			GlslToSpirv(desc, out_binaries);

		std::string cachedPath = Gfx_Helpers::GetCachedPath(desc.myFilePath, CachedPathType::Shader);
		std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
		if (out.is_open())
		{
			out.write((char*)out_binaries.data(), out_binaries.size() * sizeof(uint32_t));
			out.flush();
			out.close();
		}
	}

	void Gfx_ShaderCompiler::LoadSPIRV(const std::string& path, std::vector<uint32_t>& out_binaries)
	{
		std::ifstream in(path, std::ios::in | std::ios::binary);
		if (in.is_open())
		{
			in.seekg(0, std::ios::end);
			auto size = in.tellg();
			in.seekg(0, std::ios::beg);

			out_binaries.resize(size / sizeof(uint32_t));
			in.read((char*)out_binaries.data(), size);
		}
	}

	void Gfx_ShaderCompiler::SpirvToGlsl(const ShaderCompileDesc& desc)
	{
		std::vector<uint32_t> binaries{};
		LoadSPIRV(desc.myFilePath, binaries);

		spirv_cross::CompilerGLSL compiler(binaries);

		// Set some options.
		spirv_cross::CompilerGLSL::Options options;
		options.version = 460;
		options.vulkan_semantics = true;

		compiler.set_common_options(options);

		std::string glsl = compiler.compile();

		{
			std::ofstream file;
			file.open(desc.myFilePathOut);

			if (file.is_open())
			{
				file << glsl;
			}
			file.close();
		}
	}

	void Gfx_ShaderCompiler::SpirvToHlsl(const ShaderCompileDesc& desc)
	{
		std::vector<uint32_t> binaries{};
		LoadSPIRV(desc.myFilePath, binaries);
		spirv_cross::CompilerHLSL compiler(binaries);

		spirv_cross::CompilerHLSL::Options options{};
		options.shader_model = 66;

		compiler.set_hlsl_options(options);

		std::string hlsl = compiler.compile();
		{
			std::ofstream file;
			file.open(desc.myFilePathOut);

			if (file.is_open())
			{
				file << hlsl;
			}
			file.close();
		}
	}

	void Gfx_ShaderCompiler::GlslToSpirv(const ShaderCompileDesc& desc, std::vector<uint32_t>& out_binaries)
	{
		std::ifstream file(desc.myFilePath);
		std::stringstream buffer;

		GFX_ASSERT_MSG(file, "Could not load file")

		buffer << file.rdbuf();
		std::string src = buffer.str();
		file.close();

		// Initialize glslang library.
		glslang::InitializeProcess();

		const char* file_name_list[1] = { "" };
		const char* shader_source = reinterpret_cast<const char*>(src.data());

		EShMessages messages = static_cast<EShMessages>(EShMsgDefault | EShMsgVulkanRules | EShMsgSpvRules);
		EShLanguage language = locGetShaderType(desc.myStage);

		glslang::TShader shader(language);

		shader.setAutoMapLocations(true);
		shader.setAutoMapBindings(true);

		shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_5);
		shader.setStringsWithLengthsAndNames(&shader_source, nullptr, file_name_list, 1);
		shader.setEntryPoint("main");
		shader.setEnvInput(glslang::EShSource::EShSourceGlsl, shader.getStage(), glslang::EShClientVulkan, 100);
		shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
		shader.setSourceEntryPoint("main");

		TPreamble prebale;
		for (auto& [name, value] : desc.myDefines)
		{
			std::string res = value ? "0" : "1";
			prebale.addDef(name + "=" + res);
		}

		shader.setPreamble(prebale.get());

		Gfx_ShaderIncluder* includer = Gfx_ShaderIncluder::GetSingleton();

		TBuiltInResource Resources{};

		Resources.maxLights = 32;
		Resources.maxClipPlanes = 6;
		Resources.maxTextureUnits = 32;
		Resources.maxTextureCoords = 32;
		Resources.maxVertexAttribs = 64;
		Resources.maxVertexUniformComponents = 4096;
		Resources.maxVaryingFloats = 64;
		Resources.maxVertexTextureImageUnits = 32;
		Resources.maxCombinedTextureImageUnits = 80;
		Resources.maxTextureImageUnits = 32;
		Resources.maxFragmentUniformComponents = 4096;
		Resources.maxDrawBuffers = 32;
		Resources.maxVertexUniformVectors = 128;
		Resources.maxVaryingVectors = 8;
		Resources.maxFragmentUniformVectors = 16;
		Resources.maxVertexOutputVectors = 16;
		Resources.maxFragmentInputVectors = 15;
		Resources.minProgramTexelOffset = -8;
		Resources.maxProgramTexelOffset = 7;
		Resources.maxClipDistances = 8;
		Resources.maxComputeWorkGroupCountX = 65535;
		Resources.maxComputeWorkGroupCountY = 65535;
		Resources.maxComputeWorkGroupCountZ = 65535;
		Resources.maxComputeWorkGroupSizeX = 1024;
		Resources.maxComputeWorkGroupSizeY = 1024;
		Resources.maxComputeWorkGroupSizeZ = 64;
		Resources.maxComputeUniformComponents = 1024;
		Resources.maxComputeTextureImageUnits = 16;
		Resources.maxComputeImageUniforms = 8;
		Resources.maxComputeAtomicCounters = 8;
		Resources.maxComputeAtomicCounterBuffers = 1;
		Resources.maxVaryingComponents = 60;
		Resources.maxVertexOutputComponents = 64;
		Resources.maxGeometryInputComponents = 64;
		Resources.maxGeometryOutputComponents = 128;
		Resources.maxFragmentInputComponents = 128;
		Resources.maxImageUnits = 8;
		Resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
		Resources.maxCombinedShaderOutputResources = 8;
		Resources.maxImageSamples = 0;
		Resources.maxVertexImageUniforms = 0;
		Resources.maxTessControlImageUniforms = 0;
		Resources.maxTessEvaluationImageUniforms = 0;
		Resources.maxGeometryImageUniforms = 0;
		Resources.maxFragmentImageUniforms = 8;
		Resources.maxCombinedImageUniforms = 8;
		Resources.maxGeometryTextureImageUnits = 16;
		Resources.maxGeometryOutputVertices = 256;
		Resources.maxGeometryTotalOutputComponents = 1024;
		Resources.maxGeometryUniformComponents = 1024;
		Resources.maxGeometryVaryingComponents = 64;
		Resources.maxTessControlInputComponents = 128;
		Resources.maxTessControlOutputComponents = 128;
		Resources.maxTessControlTextureImageUnits = 16;
		Resources.maxTessControlUniformComponents = 1024;
		Resources.maxTessControlTotalOutputComponents = 4096;
		Resources.maxTessEvaluationInputComponents = 128;
		Resources.maxTessEvaluationOutputComponents = 128;
		Resources.maxTessEvaluationTextureImageUnits = 16;
		Resources.maxTessEvaluationUniformComponents = 1024;
		Resources.maxTessPatchComponents = 120;
		Resources.maxPatchVertices = 32;
		Resources.maxTessGenLevel = 64;
		Resources.maxViewports = 16;
		Resources.maxVertexAtomicCounters = 0;
		Resources.maxTessControlAtomicCounters = 0;
		Resources.maxTessEvaluationAtomicCounters = 0;
		Resources.maxGeometryAtomicCounters = 0;
		Resources.maxFragmentAtomicCounters = 8;
		Resources.maxCombinedAtomicCounters = 8;
		Resources.maxAtomicCounterBindings = 1;
		Resources.maxVertexAtomicCounterBuffers = 0;
		Resources.maxTessControlAtomicCounterBuffers = 0;
		Resources.maxTessEvaluationAtomicCounterBuffers = 0;
		Resources.maxGeometryAtomicCounterBuffers = 0;
		Resources.maxFragmentAtomicCounterBuffers = 1;
		Resources.maxCombinedAtomicCounterBuffers = 1;
		Resources.maxAtomicCounterBufferSize = 16384;
		Resources.maxTransformFeedbackBuffers = 4;
		Resources.maxTransformFeedbackInterleavedComponents = 64;
		Resources.maxCullDistances = 8;
		Resources.maxCombinedClipAndCullDistances = 8;
		Resources.maxSamples = 4;
		Resources.maxMeshOutputVerticesNV = 256;
		Resources.maxMeshOutputPrimitivesNV = 512;
		Resources.maxMeshWorkGroupSizeX_NV = 32;
		Resources.maxMeshWorkGroupSizeY_NV = 1;
		Resources.maxMeshWorkGroupSizeZ_NV = 1;
		Resources.maxTaskWorkGroupSizeX_NV = 32;
		Resources.maxTaskWorkGroupSizeY_NV = 1;
		Resources.maxTaskWorkGroupSizeZ_NV = 1;
		Resources.maxMeshViewCountNV = 4;

		Resources.limits.nonInductiveForLoops = 1;
		Resources.limits.whileLoops = 1;
		Resources.limits.doWhileLoops = 1;
		Resources.limits.generalUniformIndexing = 1;
		Resources.limits.generalAttributeMatrixVectorIndexing = 1;
		Resources.limits.generalVaryingIndexing = 1;
		Resources.limits.generalSamplerIndexing = 1;
		Resources.limits.generalVariableIndexing = 1;
		Resources.limits.generalConstantMatrixVectorIndexing = 1;


		if (!shader.parse(&Resources, 100, false, messages, *includer))
		{
			GFX_LOG(std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog()), Gfx_Log::Level::Error)
		}

		// Add shader to new program object.
		glslang::TProgram program;
		program.addShader(&shader);

		// Link program.
		if (!program.link(messages))
		{
			GFX_LOG(std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog()), Gfx_Log::Level::Error)
		}

		glslang::TIntermediate* intermediate = program.getIntermediate(language);

		GFX_ASSERT_MSG(intermediate, "Failed to get shared intermediate code");

		spv::SpvBuildLogger logger;
		glslang::SpvOptions options = {};
		options.disableOptimizer = false;

		glslang::GlslangToSpv(*intermediate, out_binaries, &logger, &options);
		std::string error = logger.getAllMessages();

		GFX_ASSERT_MSG(error.empty(), error)

		// Shutdown glslang library.
		glslang::FinalizeProcess();
	}

	void Gfx_ShaderCompiler::HlslToSpirv(const ShaderCompileDesc& desc, std::vector<uint32_t>& out_binaries)
	{
		Gfx_ShaderIncluder* includer = Gfx_ShaderIncluder::GetSingleton();
		const auto sdk_path = std::getenv("VULKAN_SDK");
		const std::string& base_path = Gfx_App::GetSingleton()->GetAssetsPath();
		const std::string  compiler_path = std::string(sdk_path) + "/Bin/dxc.exe" + " ";
		const std::string  include_path = "";
		const std::string  shader_path = std::filesystem::absolute(desc.myFilePath).string();
		const std::string  shader_path_temp = shader_path + ".temp";
		const std::string  opt_str = desc.myOptimize ? "" : "-Od ";
		const std::string  opt2_str = desc.myDebug ? "-Zi " : "";

		std::string  defines = "";
		if (desc.myDefines.size() > 0)
		{
			defines = "-D";

			for (auto& [name, value] : desc.myDefines)
			{
				std::string res = value ? "0" : "1";
				defines += " " + name + "=" + res;
			}

			defines += " ";
		}

		std::string profiler = "-T lib_6_6 ";
		switch (desc.myStage)
		{
		case ShaderStage::Compute:    profiler = "-T cs_6_6 "; break;
		case ShaderStage::Fragment:   profiler = "-T ps_6_6 "; break;
		case ShaderStage::Vertex:     profiler = "-T vs_6_6 "; break;
		case ShaderStage::Geometry:   profiler = "-T gs_6_6 "; break;
		}

		{
			std::stringstream command{};

			command << compiler_path << "-spirv " << "spirv-opt -O3 " << opt_str << defines << opt2_str << profiler << "-fspv-target-env=vulkan1.2 " << "-E main "
				<< include_path << shader_path << " -Fo " + shader_path_temp;

			assert(system(command.str().c_str()) == 0);
		}

		LoadSPIRV(shader_path_temp, out_binaries);
		std::filesystem::remove(shader_path_temp);
	}

}