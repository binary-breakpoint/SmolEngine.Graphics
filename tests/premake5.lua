project "Demo Renderer"
language "C++"
cppdialect "C++20"
staticruntime "off"

filter "configurations:Debug"
kind "ConsoleApp"
filter {}
filter "configurations:Release"
kind "ConsoleApp"
filter {}
filter "configurations:Dist"
kind "WindowedApp"
filter {}

targetdir ("bin/" .. outputdir .. "/%{prj.name}")
objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
linkoptions { "/ignore:4099" }

VULKAN_SDK = os.getenv("VULKAN_SDK")

files
{
    "src/Demo_Basic.cpp",
}

includedirs
{
    "%{VULKAN_SDK}/Include",
    
    "../include",
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
}

links
{
    "Dia"
}

filter "system:windows"
systemversion "latest"

defines
{
    "_CRT_SECURE_NO_WARNINGS",
    "PLATFORM_WIN",

    --"AFTERMATH"
}

filter "configurations:Debug"
symbols "on"

filter "configurations:Release"
optimize "full"

filter "configurations:Dist"
optimize "full"
defines "DIA_DIST"
filter {}
----------------------------------------------------------------------------------------------------------

project "Raytracing Demo"
language "C++"
cppdialect "C++20"
staticruntime "off"

filter "configurations:Debug"
kind "ConsoleApp"
filter {}
filter "configurations:Release"
kind "ConsoleApp"
filter {}
filter "configurations:Dist"
kind "WindowedApp"
filter {}

targetdir ("bin/" .. outputdir .. "/%{prj.name}")
objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
linkoptions { "/ignore:4099" }

VULKAN_SDK = os.getenv("VULKAN_SDK")

files
{
    "src/Demo_Raytracing.cpp",
}

includedirs
{
    "%{VULKAN_SDK}/Include",
    
    "../include",
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
}

links
{
    "Dia"
}

filter "system:windows"
systemversion "latest"

defines
{
    "_CRT_SECURE_NO_WARNINGS",
    "PLATFORM_WIN",

    --"AFTERMATH"
}

filter "configurations:Debug"
symbols "on"

filter "configurations:Release"
optimize "full"

filter "configurations:Dist"
optimize "full"
defines "DIA_DIST"
filter {}
-----------------------------------------------------------------

project "Compute Demo"
language "C++"
cppdialect "C++20"
staticruntime "off"

filter "configurations:Debug"
kind "ConsoleApp"
filter {}
filter "configurations:Release"
kind "ConsoleApp"
filter {}
filter "configurations:Dist"
kind "WindowedApp"
filter {}

targetdir ("bin/" .. outputdir .. "/%{prj.name}")
objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
linkoptions { "/ignore:4099" }

VULKAN_SDK = os.getenv("VULKAN_SDK")

files
{
    "src/Demo_Compute.cpp",
}

includedirs
{
    "%{VULKAN_SDK}/Include",
    
    "../include",
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
}

links
{
    "Dia"
}

filter "system:windows"
systemversion "latest"

defines
{
    "_CRT_SECURE_NO_WARNINGS",
    "PLATFORM_WIN",

    --"AFTERMATH"
}

filter "configurations:Debug"
symbols "on"

filter "configurations:Release"
optimize "full"

filter "configurations:Dist"
optimize "full"
defines "DIA_DIST"
filter {}