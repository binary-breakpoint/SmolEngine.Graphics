project "ImGui"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("lib/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	
    VULKAN_SDK = os.getenv("VULKAN_SDK")

	files
	{
		"imconfig.h",
		"imgui.h",
		"imgui.cpp",
        "imgui_demo.cpp",
		"imgui_draw.cpp",
		"imgui_internal.h",
		"imgui_widgets.cpp",
        "imgui_tables.cpp",
		"imstb_rectpack.h",
		"imstb_textedit.h",
		"imstb_truetype.h",
		"imgui_demo.cpp",

		"misc/cpp/imgui_stdlib.cpp",

		"backends/imgui_impl_glfw.cpp",
		"backends/imgui_impl_vulkan.cpp",
	}

	includedirs
	{
		"",
		"%{VULKAN_SDK}/Include",
		"../glfw/include",
		"../"
	}

	filter "system:windows"
		systemversion "latest"

		filter "configurations:Debug"
		symbols "on"
		defines "SMOLENGINE_DEBUG"

		filter "configurations:Release"
		optimize "full"
		defines "SMOLENGINE_DEBUG"

		filter "configurations:Dist"
		optimize "full"
		defines "SMOLENGINE_DIST"