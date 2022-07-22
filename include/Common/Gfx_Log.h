#pragma once
#include "Common/Gfx_Memory.h"

#include <sstream>
#include <mutex>
#include <source_location>

extern "C++"
{
#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/stdout_color_sinks.h>
}

namespace Dia
{
	using source_location = std::source_location;
	[[nodiscard]] constexpr auto get_log_source_location(
		const source_location& location) {
		return spdlog::source_loc{ location.file_name(),
								  static_cast<std::int32_t>(location.line()),
								  location.function_name() };
	}

	struct format_with_location {
		std::string_view value;
		spdlog::source_loc loc;

		template <typename String>
		format_with_location(
			const String& s,
			const source_location& location = source_location::current())
			: value{ s }, loc{ get_log_source_location(location) } {}
	};

	enum class LogLevel
	{
		Info,
		Warning,
		Error
	};

	class Gfx_Log
	{
	public:
		Gfx_Log();
		~Gfx_Log();

		template<typename... Args>
		static void LogError(const format_with_location& fmt, Args&& ...args)
		{
			Log(LogLevel::Error, fmt, std::forward<Args>(args)...);
		}

		template<typename... Args>
		static void LogInfo(const format_with_location& fmt, Args&& ...args)
		{
			Log(LogLevel::Info, fmt, std::forward<Args>(args)...);
		}

		template<typename... Args>
		static void LogWarn(const format_with_location& fmt, Args&& ...args)
		{
			Log(LogLevel::Warning, fmt, std::forward<Args>(args)...);
		}

		template<typename... Args>
		static void Log(LogLevel level, const format_with_location& fmt, Args&& ...args)
		{
			const std::lock_guard<std::mutex> lock(s_Instance->m_Mutex);

			spdlog::level::level_enum spdLevel = spdlog::level::trace;

			switch (level)
			{
			case LogLevel::Warning:
			{
				spdLevel = spdlog::level::warn;
				break;
			}
			case LogLevel::Error:
			{
				spdLevel = spdlog::level::err;
				break;
			}
			case LogLevel::Info:
			{
				spdlog::level::trace;
				break;
			}
			}

			s_Instance->m_Logger->log(fmt.loc, spdLevel, fmt.value, std::forward<Args>(args)...);

			const auto& callback = s_Instance->m_Callback;
			if (callback != nullptr)
			{
				// temp
			}
		}

		static void SetCallback(const std::function<void(const std::string&&, LogLevel)>& callback)
		{
			s_Instance->m_Callback = callback;
		}

	public:
		static Gfx_Log* s_Instance;
		std::mutex m_Mutex{};
		std::shared_ptr<spdlog::logger> m_Logger = nullptr;
		std::function<void(const std::string&&, LogLevel)> m_Callback = nullptr;
	};

#define RUNTIME_ERROR(msg, ...) Gfx_Log::LogError(msg, __VA_ARGS__); abort()
}