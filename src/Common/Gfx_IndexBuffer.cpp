#include "Gfx_Precompiled.h"
#include "Common/Gfx_IndexBuffer.h"

namespace SmolEngine
{
	static VkBufferUsageFlags locGetIndexBufferUsageFlags()
	{
		VkBufferUsageFlags flags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		if (Gfx_Context::GetDevice().GetRaytracingSupport())
		{
			flags |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
		}

		return flags;
	}

	Gfx_IndexBuffer::Gfx_IndexBuffer() :
		m_Elements{ 0 } {}

	bool Gfx_IndexBuffer::Create(uint32_t* indices, size_t count, bool isStatic)
	{
		VkBufferUsageFlags flags = locGetIndexBufferUsageFlags();

		BufferCreateDesc desc{};
		desc.myData = indices;
		desc.mySize = sizeof(uint32_t) * count;
		desc.myFlags = isStatic ? BufferCreateDesc::CreateFlags::Static : BufferCreateDesc::CreateFlags::Default;
		desc.myBufferUsage = flags;

		m_Buffer.Create(desc);
		m_Elements = static_cast<uint32_t>(count);

		return m_Elements > 0;
	}

	bool Gfx_IndexBuffer::Create(size_t size, bool isStatic)
	{
		return Create(nullptr, isStatic);
	}

	void Gfx_IndexBuffer::Update(uint32_t* indices, size_t count, uint32_t offset /*= 0*/)
	{
		m_Buffer.SetData(indices, sizeof(uint32_t) * count, offset);
	}

	void Gfx_IndexBuffer::Free()
	{
		m_Buffer.Free();
		m_Elements = 0;
	}

	bool Gfx_IndexBuffer::IsGood() const
	{
		return m_Elements > 0;
	}

	const Gfx_Buffer& Gfx_IndexBuffer::GetBuffer() const
	{
		return m_Buffer;
	}

	uint32_t Gfx_IndexBuffer::GetCount() const
	{
		return m_Elements;
	}

}