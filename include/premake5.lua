group "Dependencies"
include "../vendor/glfw"
include "../vendor/imgizmo"
include "../vendor/imgui"
include "../vendor/ozz-animation"
group ""

group "Engine"
project "SmolEngine.Graphics"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "Gfx_Precompiled.h"
	pchsource "../src/Gfx_Precompiled.cpp"
	linkoptions { "/ignore:4006" }

	VULKAN_SDK = os.getenv("VULKAN_SDK")

	files
	{
		"../src/**.cpp",
		"../src/**.h",

		"**.h",

		"../vendor/stb_image/**.h",
		"../vendor/vulkan_memory_allocator/vk_mem_alloc.h",

		"../vendor/implot/**.cpp",
		"../vendor/vulkan_memory_allocator/vk_mem_alloc.cpp",
	}

	includedirs
	{
		"../include/",
		"../src/",

		"../vendor/",
		"../vendor/implot/",
		"../vendor/spdlog/include",
		"../vendor/glm",
		"../vendor/imgui",
		"../vendor/imgizmo/src",
		"../vendor/stb_image",

		"../vendor/nvidia_aftermath/include",
		"../vendor/ozz-animation/include",
		"../vendor/cereal/include",
		"../vendor/glfw/include",
		"../vendor/tinygltf",
		"../vendor/gli",

		"%{VULKAN_SDK}/Include",
		"%{VULKAN_SDK}/Include/glslang/include",
	}

	links 
	{ 
		"GLFW",
		"ImGizmo",
		"ImGui",
		"Ozz-Animation",

		"%{VULKAN_SDK}/Lib/vulkan-1.lib",
	}

	defines
	{
        "_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",

		--"AFTERMATH"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"PLATFORM_WIN",
			"GLFW_INCLUDE_NONE"
		}

	filter "configurations:Debug"
		symbols "on"

		links 
		{ 
			"%{VULKAN_SDK}/Lib/VkLayer_utils.lib",

			"%{VULKAN_SDK}/Lib/spirv-cross-glsld.lib",
			"%{VULKAN_SDK}/Lib/spirv-cross-hlsld.lib",
			"%{VULKAN_SDK}/Lib/spirv-cross-cored.lib",

			"%{VULKAN_SDK}/Lib/OGLCompilerd.lib",
			"%{VULKAN_SDK}/Lib/OSDependentd.lib",
			"%{VULKAN_SDK}/Lib/SPIRV-Toolsd.lib",
			"%{VULKAN_SDK}/Lib/SPIRV-Tools-optd.lib",

			"%{VULKAN_SDK}/Lib/glslang-default-resource-limitsd.lib",
			"%{VULKAN_SDK}/Lib/glslangd.lib",
			"%{VULKAN_SDK}/Lib/GenericCodeGend.lib",
			"%{VULKAN_SDK}/Lib/MachineIndependentd.lib",
			"%{VULKAN_SDK}/Lib/SPVRemapperd.lib",
			"%{VULKAN_SDK}/Lib/SPIRVd.lib",

			"../vendor/nvidia_aftermath/lib/GFSDK_Aftermath_Lib.x64.lib",
			"../vendor/nvidia_aftermath/lib/GFSDK_Aftermath_Lib_UWP.x64.lib",
		}

		defines
		{
			"DIA_DEBUG"
		}

	filter "configurations:Release"
	optimize "full"

		links 
		{ 
			"%{VULKAN_SDK}/Lib/spirv-cross-glsl.lib",
			"%{VULKAN_SDK}/Lib/spirv-cross-hlsl.lib",
			"%{VULKAN_SDK}/Lib/spirv-cross-core.lib",

			"%{VULKAN_SDK}/Lib/SPIRV-Tools.lib",
			"%{VULKAN_SDK}/Lib/SPIRV-Tools-opt.lib",
			"%{VULKAN_SDK}/Lib/OGLCompiler.lib",
			"%{VULKAN_SDK}/Lib/OSDependent.lib",

			"%{VULKAN_SDK}/Lib/glslang-default-resource-limits.lib",
			"%{VULKAN_SDK}/Lib/glslang.lib",
			"%{VULKAN_SDK}/Lib/GenericCodeGen.lib",
			"%{VULKAN_SDK}/Lib/MachineIndependent.lib",
			"%{VULKAN_SDK}/Lib/SPVRemapper.lib",
			"%{VULKAN_SDK}/Lib/SPIRV.lib",
		}

		filter "configurations:Dist"
		optimize "full"
		defines "DIA_DIST"

		links 
		{ 
			"%{VULKAN_SDK}/Lib/spirv-cross-glsl.lib",
			"%{VULKAN_SDK}/Lib/spirv-cross-hlsl.lib",
			"%{VULKAN_SDK}/Lib/spirv-cross-core.lib",

			"%{VULKAN_SDK}/Lib/SPIRV-Tools.lib",
			"%{VULKAN_SDK}/Lib/SPIRV-Tools-opt.lib",
			"%{VULKAN_SDK}/Lib/OGLCompiler.lib",
			"%{VULKAN_SDK}/Lib/OSDependent.lib",

			"%{VULKAN_SDK}/Lib/glslang-default-resource-limits.lib",
			"%{VULKAN_SDK}/Lib/glslang.lib",
			"%{VULKAN_SDK}/Lib/GenericCodeGen.lib",
			"%{VULKAN_SDK}/Lib/MachineIndependent.lib",
			"%{VULKAN_SDK}/Lib/SPVRemapper.lib",
			"%{VULKAN_SDK}/Lib/SPIRV.lib",
		}

group ""