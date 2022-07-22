project "ImGizmo"
    kind "StaticLib"
    language "C"
    staticruntime "off"
    
    targetdir ("../libs/" .. outputdir .. "/%{prj.name}")
    objdir ("../libs/bin-int/" .. outputdir .. "/%{prj.name}")

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
	
		filter "configurations:Release"
		optimize "full"
		
		filter "configurations:Dist"
		optimize "full"
		defines "DIA_DIST"

	

