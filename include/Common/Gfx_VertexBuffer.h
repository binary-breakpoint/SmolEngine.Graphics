#pragma once
#include "Common/Gfx_Asset.h"
#include "Common/Gfx_Buffer.h"

namespace SmolEngine
{
	class Gfx_VertexBuffer final: public Gfx_Asset
	{
	public:
		Gfx_VertexBuffer();

		void Free() override;
		bool IsGood() const override;
		bool Create(void* vertices, size_t size, uint32_t vertexCount, bool is_static = false);
		bool Create(size_t size, uint32_t vertexCount, bool is_static = false);
		void Update(const void* data, size_t size, const uint32_t offset = 0);
		Gfx_Buffer& GetBuffer();
		uint32_t GetVertexCount() const { return m_VertexCount; }

	private:
		Gfx_Buffer m_Buffer;
		uint32_t m_VertexCount;
	};
}