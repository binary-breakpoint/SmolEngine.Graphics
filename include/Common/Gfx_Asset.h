#pragma once
#include "Gfx_Memory.h"

namespace SmolEngine
{
	class Gfx_Asset
	{
	public:
		virtual ~Gfx_Asset()
		{
			Free();
		}

		virtual void Free() {};
		virtual bool IsGood() const { return false; };
	};
}