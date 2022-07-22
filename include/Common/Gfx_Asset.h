#pragma once
#include "Gfx_Memory.h"

namespace Dia
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