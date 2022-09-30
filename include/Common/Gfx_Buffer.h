#pragma once
#include "Backend/Gfx_VulkanCore.h"
#include "Backend/Gfx_VulkanAllocator.h"

namespace SmolEngine
{
	struct BufferCreateDesc
	{
		enum class CreateFlags
		{
			Default,
			Static,
			Scratch,
			Staging,
		};

		BufferCreateDesc();

		void* myData;
		size_t mySize;
		VkBufferUsageFlags myBufferUsage;
		VkSharingMode mySharingMode;
		VmaMemoryUsage myMemUsage;
		CreateFlags myFlags;
	};

	class Gfx_Buffer
	{
	public:
		Gfx_Buffer();
		~Gfx_Buffer();

		void Free();
		void Create(const BufferCreateDesc& desc);
		void SetData(const void* data, size_t size, uint32_t offset = 0);
		void ResetBufferUint(VkCommandBuffer cmd, uint32_t value);
		void* MapMemory();
		void UnMapMemory();

		bool IsGood() const;
		size_t GetSize() const;
		VkBuffer GetRawBuffer() const;
		size_t GetOffset() const;
		VkBufferView GetVkBufferView() const;
		VmaAllocation GetVmaAllocation() const;
		uint64_t GetDeviceAddress() const;
		VkBufferUsageFlags GetBufferFlags() const;

	private:
		void* m_Mapped;
		VkBuffer m_Buffer;
		VkBufferView m_BufferView;
		VmaAllocation m_Alloc;
		size_t m_Size;
		size_t m_Offset;
		uint64_t m_DeviceAddress;
		VkBufferUsageFlags m_Usage;
	};
}