#pragma once
#include "Common/Gfx_Helpers.h"

namespace Dia
{
	struct Gfx_BufferElement
	{
		Gfx_BufferElement(Format format, const std::string& name)
			:m_Name(name), m_Format(format), m_Size(Gfx_Helpers::GetFormatSize(format)), m_Offset(0) {}

		uint32_t m_Size;
		uint32_t m_Offset;
		Format m_Format;
		std::string m_Name;
	};
}