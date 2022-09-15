project "ImGizmo"
    kind "StaticLib"
    language "C"
    staticruntime "off"
    
    targetdir ("lib/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
		"src/**.cpp",
		"src/**.h",
    }

	includedirs
	{
		"../../vendor/",
		"src/"
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
	

