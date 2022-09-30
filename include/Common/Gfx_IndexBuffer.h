#pragma once
#include "Common/Gfx_Buffer.h"

namespace SmolEngine
{
	class Gfx_IndexBuffer
	{
	public:
		Gfx_IndexBuffer();
		
		void Free();
		bool Create(uint32_t* indices, size_t count, bool isStatic = false);
		bool Create(size_t size, bool isStatic = false);
		void Update(uint32_t* indices, size_t count, uint32_t offset = 0);

		bool IsGood() const;
		const Gfx_Buffer& GetBuffer() const;
		uint32_t  GetCount() const;
		
	private:
		Gfx_Buffer m_Buffer;
		uint32_t m_Elements;
	};
}