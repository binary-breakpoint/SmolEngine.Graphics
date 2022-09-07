#include "Gfx_Precompiled.h"
#include "Common/Gfx_VertexBuffer.h"

namespace SmolEngine
{
	static VkBufferUsageFlags locGetIndexBufferUsageFlags()
	{
		VkBufferUsageFlags flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		if (Gfx_Context::GetDevice().GetRaytracingSupport())
		{
			flags |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
		}

		return flags;
	}

	Gfx_VertexBuffer::Gfx_VertexBuffer()
		:m_VertexCount{0}
	{

	}

	bool Gfx_VertexBuffer::Create(void* vertices, size_t size, uint32_t vertexCount, bool isStatic)
	{
		VkBufferUsageFlags flags = locGetIndexBufferUsageFlags();

		BufferCreateDesc desc{};
		desc.myData = vertices;
		desc.mySize = size;
		desc.myFlags = isStatic ? BufferCreateDesc::CreateFlags::Static : BufferCreateDesc::CreateFlags::Default;
		desc.myBufferUsage = flags;

		m_Buffer.Create(desc);
		m_VertexCount = vertexCount;

		return m_VertexCount > 0;
	}

	bool Gfx_VertexBuffer::Create(size_t size, uint32_t vertexCount, bool isStatic)
	{
		return Create(nullptr, size, vertexCount, isStatic);
	}

	void Gfx_VertexBuffer::Update(const void* data, size_t size, const uint32_t offset)
	{
		m_Buffer.SetData(data, size, offset);
	}

	void Gfx_VertexBuffer::Free()
	{
		m_Buffer.Free();
		m_VertexCount = 0;
	}

	bool Gfx_VertexBuffer::IsGood() const
	{
		return m_VertexCount > 0;
	}

	Gfx_Buffer& Gfx_VertexBuffer::GetBuffer()
	{
		return m_Buffer;
	}
}