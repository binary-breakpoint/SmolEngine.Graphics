#pragma once
#include "Common/Gfx_Memory.h"

#include <functional>
#include <utility>
#include <format>

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

		template<typename String, typename... Args>
		static void Log(Level level, const String& string, Args&& ...args)
		{
#ifdef SMOLENGINE_DEBUG
			std::string msg = std::vformat(string, std::make_format_args(args...));
			const auto& callback = s_Instance->m_Callback;
			if (callback != nullptr)
				callback(msg, level);
#endif
		}

		static void SetCallback(const std::function<void(const std::string&, Level)>& callback)
		{
#ifdef SMOLENGINE_DEBUG
			s_Instance->m_Callback = callback;
#endif
		}

		static Gfx_Log* s_Instance;

	private:
		std::function<void(const std::string&, Level)> m_Callback;
	};

#ifdef SMOLENGINE_DEBUG
#define GFX_ASSERT_MSG(condition, msg)	if(!condition) { Gfx_Log::Log(Gfx_Log::Level::Error, msg); assert(condition); }
#define GFX_ASSERT(condition) assert(condition);
#define GFX_LOG(msg, level, ...) Gfx_Log::Log(level, msg, __VA_ARGS__);
#else
#define GFX_ASSERT_MSG(condition, msg)
#define GFX_ASSERT(condition)
#define GFX_LOG(msg, level)	
#endif

}