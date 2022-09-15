
workspace "SmolEngine.Graphics"
architecture "x64"
startproject "Compute Demo"

configurations
{
	"Debug",
	"Release",
	"Dist"
}

flags
{
	"MultiProcessorCompile"
}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "include/"

group "Executables"
include "tests"
group ""