project "Ozz-Animation"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

	targetdir ("../libs/" .. outputdir .. "/%{prj.name}")
	objdir ("../libs/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.cc",
		"include/**.h",
	}

	includedirs
	{
		"src/",
		"include/",
		"extern/"
	}

	filter "system:windows"
		systemversion "latest"

		filter "configurations:Debug"
		symbols "on"
	
		filter "configurations:Release"
		optimize "full"
		
		filter "configurations:Dist"
		optimize "full"
		defines "DIA_DIST"


