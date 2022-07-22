#pragma once
#include "Common/Gfx_BufferElement.h"

#include <vector>

namespace Dia
{
	class Gfx_BufferLayout
	{
	public:
		Gfx_BufferLayout() = default;
		Gfx_BufferLayout(const std::initializer_list<Gfx_BufferElement>& elemets)
			:m_Elements(elemets) {
			CalculateOffsetAndPride();
		}

		std::vector<Gfx_BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<Gfx_BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<Gfx_BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<Gfx_BufferElement>::const_iterator end() const { return m_Elements.end(); }

		uint32_t GetStride() const { return m_Stride; }
		const std::vector<Gfx_BufferElement>& GetElements() const { return m_Elements; }

	private:
		void CalculateOffsetAndPride()
		{
			uint32_t offset = 0;
			m_Stride = 0;

			for (auto& element : m_Elements)
			{
				element.m_Offset = offset;
				offset += element.m_Size;
				m_Stride += element.m_Size;
			}
		}

	private:
		uint32_t m_Stride;
		std::vector<Gfx_BufferElement> m_Elements;
	};
}