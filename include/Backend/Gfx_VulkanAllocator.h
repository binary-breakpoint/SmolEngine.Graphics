#pragma once
#include "Backend/Gfx_VulkanCore.h"

VK_DEFINE_HANDLE(VmaAllocation)
VK_DEFINE_HANDLE(VmaAllocator)

struct VmaAllocationInfo;
enum VmaMemoryUsage;

namespace SmolEngine
{
	class Gfx_VulkanDevice;
	class Gfx_VulkanInstance;

	class Gfx_VulkanAllocator
	{
	public:
		Gfx_VulkanAllocator();
		~Gfx_VulkanAllocator();

		void Init(Gfx_VulkanDevice* device, Gfx_VulkanInstance* instance);

		static VmaAllocation AllocBuffer(VkBufferCreateInfo ci, VmaMemoryUsage usage, VkBuffer& outBuffer);
		static VmaAllocation AllocImage(VkImageCreateInfo ci, VmaMemoryUsage usage, VkImage& outImage);

		static void AllocFree(VmaAllocation allocation);
		static void FreeImage(VkImage image, VmaAllocation allocation);
		static void FreeBuffer(VkBuffer buffer, VmaAllocation allocation);

		static void UnmapMemory(VmaAllocation allocation);
		static uint8_t* MapMemory(VmaAllocation allocation);

		static void GetAllocInfo(VmaAllocation allocation, VmaAllocationInfo*& outInfo);

		static Gfx_VulkanAllocator* s_Instance;

		VmaAllocator m_Allocator;
		uint64_t m_TotalAllocatedBytes;
	};
}