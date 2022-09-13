#include "Gfx_Precompiled.h"
#include "Common/Gfx_Log.h"

namespace SmolEngine
{
	Gfx_Log* Gfx_Log::s_Instance = new Gfx_Log();

	Gfx_Log::Gfx_Log()
		:
		m_Callback{nullptr}
	{
		s_Instance = this;
	}

	Gfx_Log::~Gfx_Log()
	{
		s_Instance = nullptr;
	}
}