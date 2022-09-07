#include "Gfx_Precompiled.h"
#include "Common/Gfx_Buffer.h"
#include "Common/Gfx_CmdBuffer.h"

#include "Backend/Gfx_VulkanHelpers.h"

namespace SmolEngine
{
	Gfx_Buffer::Gfx_Buffer() :
		m_Mapped{nullptr},
		m_Buffer{nullptr},
		m_BufferView{nullptr},
		m_Alloc{nullptr},
		m_Size{0},
		m_Offset{0},
		m_DeviceAddress{0} {}

	Gfx_Buffer::~Gfx_Buffer()
	{
		Free();
	}

	void Gfx_Buffer::Create(const BufferCreateDesc& desc)
	{
		VkDevice device = Gfx_Context::GetDevice().GetLogicalDevice();

		switch (desc.myFlags)
		{
		case BufferCreateDesc::CreateFlags::Default:
		{
			VkBufferCreateInfo bufferCI = {};
			bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCI.size = desc.mySize;
			bufferCI.usage = desc.myBufferUsage;
			bufferCI.sharingMode = desc.mySharingMode;

			m_Alloc = Gfx_VulkanAllocator::AllocBuffer(bufferCI, desc.myMemUsage, m_Buffer);
			m_Size = desc.mySize;
			m_Usage = desc.myBufferUsage;

			if (desc.myData != nullptr)
				SetData(desc.myData, desc.mySize);

			break;
		}

		case BufferCreateDesc::CreateFlags::Staging:
		{

			VkBufferCreateInfo bufferCI = {};
			bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCI.size = desc.mySize;
			bufferCI.sharingMode = desc.mySharingMode;
			bufferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

			m_Alloc = Gfx_VulkanAllocator::AllocBuffer(bufferCI, VMA_MEMORY_USAGE_CPU_TO_GPU, m_Buffer);
			m_Size = desc.mySize;
			m_Usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

			SetData(desc.myData, desc.mySize);

			break;
		}

		case BufferCreateDesc::CreateFlags::Static:
		{
			BufferCreateDesc stagingDesc{};
			stagingDesc.myData = desc.myData;
			stagingDesc.mySize = desc.mySize;
			stagingDesc.myFlags = BufferCreateDesc::CreateFlags::Staging;

			Gfx_Buffer stagingBuffer{};
			stagingBuffer.Create(stagingDesc);

			VkBufferCreateInfo bufferCI = {};
			bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCI.size = desc.mySize;
			bufferCI.usage = desc.myBufferUsage;
			bufferCI.sharingMode = desc.mySharingMode;

			m_Alloc = Gfx_VulkanAllocator::AllocBuffer(bufferCI, VMA_MEMORY_USAGE_GPU_ONLY, m_Buffer);
			m_Size = desc.mySize;
			m_Usage = desc.myBufferUsage;

			Gfx_CmdBuffer cmdBuffer{};
			CmdBufferCreateDesc cmdDesc{};
			cmdBuffer.Create(&cmdDesc);

			cmdBuffer.CmdBeginRecord();
			{
				VkBufferCopy copyRegion = { };
				copyRegion.size = m_Size;
				vkCmdCopyBuffer(
					cmdBuffer.GetBuffer(),
					stagingBuffer.GetRawBuffer(),
					m_Buffer,
					1,
					&copyRegion);
			}
			cmdBuffer.CmdEndRecord();

			break;
		}
		case BufferCreateDesc::CreateFlags::Scratch:
		{
			VkBufferCreateInfo bufferInfo = {};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = desc.mySize;
			bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

			m_Alloc = Gfx_VulkanAllocator::AllocBuffer(bufferInfo, VMA_MEMORY_USAGE_GPU_ONLY, m_Buffer);

			VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{};
			bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
			bufferDeviceAddressInfo.buffer = m_Buffer;

			m_Size = desc.mySize;
			m_Usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
			m_DeviceAddress = Gfx_VulkanHelpers::GetBufferDeviceAddress(m_Buffer);

			break;
		}
		} 

		if(desc.myBufferUsage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
			m_DeviceAddress = Gfx_VulkanHelpers::GetBufferDeviceAddress(m_Buffer);
	}

	void Gfx_Buffer::Free()
	{
		if (m_Alloc != nullptr)
		{
			Gfx_VulkanAllocator::FreeBuffer(m_Buffer, m_Alloc);

			m_Size = 0;
			m_Alloc = nullptr;
			m_Mapped = nullptr;
			m_Buffer = nullptr;
		}
	}

	bool Gfx_Buffer::IsGood() const
	{
		return m_Alloc != nullptr;
	}

	void Gfx_Buffer::SetData(const void* data, size_t size, uint32_t offset)
	{
		void* dest = MapMemory();
		{
			memcpy(dest, data, size);
		}
		UnMapMemory();
	}

	void Gfx_Buffer::ResetBufferUint(VkCommandBuffer cmd, uint32_t value)
	{
		vkCmdFillBuffer(cmd, m_Buffer, 0, m_Size, value);
	}

	void* Gfx_Buffer::MapMemory()
	{
		uint8_t* destData = Gfx_VulkanAllocator::MapMemory<uint8_t>(m_Alloc);
		m_Mapped = destData;
		return m_Mapped;
	}

	void Gfx_Buffer::UnMapMemory()
	{
		if (m_Mapped != nullptr)
		{
			Gfx_VulkanAllocator::UnmapMemory(m_Alloc);
			m_Mapped = nullptr;
		}
	}

	size_t Gfx_Buffer::GetSize() const
	{
		return m_Size;
	}

	VkBuffer Gfx_Buffer::GetRawBuffer() const
	{
		return m_Buffer;
	}

	size_t Gfx_Buffer::GetOffset() const
	{
		return m_Offset;
	}

	VkBufferView Gfx_Buffer::GetVkBufferView() const
	{
		return m_BufferView;
	}

	VmaAllocation Gfx_Buffer::GetVmaAllocation() const
	{
		return m_Alloc;
	}

	uint64_t Gfx_Buffer::GetDeviceAddress() const
	{
		return m_DeviceAddress;
	}

	VkBufferUsageFlags Gfx_Buffer::GetBufferFlags() const
	{
		return m_Usage;
	}

}