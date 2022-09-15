#pragma once
#include "Common/Gfx_Asset.h"
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

	class Gfx_Buffer final: public Gfx_Asset
	{
	public:
		Gfx_Buffer();
		~Gfx_Buffer();

		void Create(const BufferCreateDesc& desc);
		void Free() override;
		bool IsGood() const override;
		void SetData(const void* data, size_t size, uint32_t offset = 0);
		void* MapMemory();
		void UnMapMemory();
		void ResetBufferUint(VkCommandBuffer cmd, uint32_t value);
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