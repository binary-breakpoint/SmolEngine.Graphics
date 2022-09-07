#include "Gfx_Precompiled.h"
#include "Common/Gfx_Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace SmolEngine
{
	Gfx_Log* Gfx_Log::s_Instance = new Gfx_Log();

	Gfx_Log::Gfx_Log()
	{
		s_Instance = this;

		spdlog::set_pattern("%^[%T] %n: %v%$");

		s_Instance->m_Logger = spdlog::stdout_color_mt("Engine");
		s_Instance->m_Logger->set_level(spdlog::level::trace);
	}

	Gfx_Log::~Gfx_Log()
	{
		s_Instance = nullptr;
	}
}