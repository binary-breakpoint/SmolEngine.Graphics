#pragma once
#include "Common/Gfx_Asset.h"
#include "Common/Gfx_Buffer.h"

namespace SmolEngine
{
	class Gfx_IndexBuffer final: public Gfx_Asset
	{
	public:
		Gfx_IndexBuffer();

		bool Create(uint32_t* indices, size_t count, bool isStatic = false);
		bool Create(size_t size, bool isStatic = false);
		void Update(uint32_t* indices, size_t count, uint32_t offset = 0);
		void Free() override;
		bool IsGood() const override;
		const Gfx_Buffer& GetBuffer() const;
		uint32_t  GetCount() const;
		
	private:
		Gfx_Buffer m_Buffer;
		uint32_t m_Elements;
	};
}