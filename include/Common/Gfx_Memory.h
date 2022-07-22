#pragma once
#include <memory>

namespace Dia
{
	extern "C++"
	{
		template<typename T>
		using Scope = std::unique_ptr<T>;

		template<typename T>
		using Ref = std::shared_ptr<T>;
	}
}