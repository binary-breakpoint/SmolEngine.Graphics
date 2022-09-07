#pragma once
#include "Backend/Gfx_VulkanCore.h"

#include <vulkan_memory_allocator/vk_mem_alloc.h>

namespace SmolEngine
{
	class Gfx_VulkanDevice;
	class Gfx_VulkanInstance;

	class Gfx_VulkanAllocator
	{
	public:
		Gfx_VulkanAllocator();
		~Gfx_VulkanAllocator();

		void                              Init(Gfx_VulkanDevice* device, Gfx_VulkanInstance* instance);
		static VmaAllocation              AllocBuffer(VkBufferCreateInfo ci, VmaMemoryUsage usage, VkBuffer& outBuffer);
		static VmaAllocation              AllocImage(VkImageCreateInfo ci, VmaMemoryUsage usage, VkImage& outImage);
		static void                       AllocFree(VmaAllocation allocation);
		static void                       FreeImage(VkImage image, VmaAllocation allocation);
		static void                       FreeBuffer(VkBuffer buffer, VmaAllocation allocation);
		static void                       UnmapMemory(VmaAllocation allocation);
		static void                       GetAllocInfo(VmaAllocation allocation, VmaAllocationInfo& outInfo);
		static VmaAllocator&              GetAllocator();

		template<typename T>
		static T* MapMemory(VmaAllocation allocation)
		{
			T* mappedMemory;
			vmaMapMemory(Gfx_VulkanAllocator::GetAllocator(), allocation, (void**)&mappedMemory);
			return mappedMemory;
		}
	private:
		static Gfx_VulkanAllocator* s_Instance;
		VmaAllocator m_Allocator;
		uint64_t m_TotalAllocatedBytes;
	};
}