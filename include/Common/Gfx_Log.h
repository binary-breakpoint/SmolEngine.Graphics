#pragma once
#include "Common/Gfx_Memory.h"

#include <functional>
#include <utility>

namespace SmolEngine
{
	class Gfx_Log
	{
	public:
		enum class Level
		{
			Info,
			Warning,
			Error
		};

		Gfx_Log();
		~Gfx_Log();

		template<typename String>
		static void Log(Level level, const String& fmt)
		{
#ifdef SMOLENGINE_DEBUG
			const auto& callback = s_Instance->m_Callback;
			if (callback != nullptr)
				callback(std::forward<const std::string>(fmt), level);
#endif
		}

		static void SetCallback(const std::function<void(const std::string&&, Level)>& callback)
		{
			s_Instance->m_Callback = callback;
		}

		static Gfx_Log* s_Instance;

	private:
		std::function<void(const std::string&&, Level)> m_Callback;
	};

#ifdef SMOLENGINE_DEBUG
#define GFX_ASSERT(condition, msg)	if(!condition) { Gfx_Log::Log(Gfx_Log::Level::Error, msg); assert(condition); }
#define GFX_ASSERT_PURE(condition) assert(condition);
#define GFX_LOG(msg, level) Gfx_Log::Log(level, msg);
#else
#define GFX_ASSERT(condition, msg)
#define GFX_ASSERT_PURE(condition)
#define GFX_LOG(msg, level)	
#endif

}