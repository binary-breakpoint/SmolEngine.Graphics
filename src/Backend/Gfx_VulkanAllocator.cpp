#include "Gfx_Precompiled.h"
#include "Backend/Gfx_VulkanAllocator.h"
#include "Backend/Gfx_VulkanDevice.h"
#include "Backend/Gfx_VulkanInstance.h"

namespace SmolEngine
{
	Gfx_VulkanAllocator* Gfx_VulkanAllocator::s_Instance = nullptr;

	Gfx_VulkanAllocator::Gfx_VulkanAllocator() :
		m_Allocator{ nullptr },
		m_TotalAllocatedBytes{ 0 }
	{
		s_Instance = this;
	}

	Gfx_VulkanAllocator::~Gfx_VulkanAllocator()
	{
		vmaDestroyAllocator(m_Allocator);
		s_Instance = nullptr;
	}

	void Gfx_VulkanAllocator::Init(Gfx_VulkanDevice* device, Gfx_VulkanInstance* instance)
	{
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
		allocatorInfo.physicalDevice = device->GetPhysicalDevice();
		allocatorInfo.device = device->GetLogicalDevice();
		allocatorInfo.instance = instance->GetInstance();

		if(device->GetRaytracingSupport())
			allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

		vmaCreateAllocator(&allocatorInfo, &m_Allocator);
	}

	VmaAllocation Gfx_VulkanAllocator::AllocBuffer(VkBufferCreateInfo ci, VmaMemoryUsage usage, VkBuffer& outBuffer)
	{
		VmaAllocationCreateInfo allocCreateInfo = {};
		allocCreateInfo.usage = usage;

		VmaAllocation allocation;
		vmaCreateBuffer(s_Instance->m_Allocator, &ci, &allocCreateInfo, &outBuffer, &allocation, nullptr);

		VmaAllocationInfo allocInfo{};
		vmaGetAllocationInfo(s_Instance->m_Allocator, allocation, &allocInfo);

#ifdef DIA_DEBUG
		s_Instance->m_TotalAllocatedBytes += allocInfo.size;
		Gfx_Log::LogInfo("[VMA]: allocating buffer; size = {}, pool size = {}", allocInfo.size, s_Instance->m_TotalAllocatedBytes);
#endif 
		return allocation;
	}

	VmaAllocation Gfx_VulkanAllocator::AllocImage(VkImageCreateInfo ci, VmaMemoryUsage usage, VkImage& outImage)
	{
		VmaAllocationCreateInfo allocCreateInfo = {};
		allocCreateInfo.usage = usage;

		VmaAllocation allocation;
		vmaCreateImage(s_Instance->m_Allocator, &ci, &allocCreateInfo, &outImage, &allocation, nullptr);
		VmaAllocationInfo allocInfo;
		vmaGetAllocationInfo(s_Instance->m_Allocator, allocation, &allocInfo);

#ifdef DIA_DEBUG
		s_Instance->m_TotalAllocatedBytes += allocInfo.size;
		Gfx_Log::LogInfo("[VMA]: allocating image; size = {}, pool size = {}", allocInfo.size, s_Instance->m_TotalAllocatedBytes);
#endif
		return allocation;
	}

	void Gfx_VulkanAllocator::AllocFree(VmaAllocation allocation)
	{
#ifdef DIA_DEBUG
		VmaAllocationInfo allocInfo;
		vmaGetAllocationInfo(s_Instance->m_Allocator, allocation, &allocInfo);
		s_Instance->m_TotalAllocatedBytes -= allocInfo.size;
#endif
		vmaFreeMemory(s_Instance->m_Allocator, allocation);
	}

	void Gfx_VulkanAllocator::FreeImage(VkImage image, VmaAllocation allocation)
	{
#ifdef DIA_DEBUG
		VmaAllocationInfo allocInfo;
		vmaGetAllocationInfo(s_Instance->m_Allocator, allocation, &allocInfo);
		s_Instance->m_TotalAllocatedBytes -= allocInfo.size;
#endif
		vmaDestroyImage(s_Instance->m_Allocator, image, allocation);
	}

	void Gfx_VulkanAllocator::FreeBuffer(VkBuffer buffer, VmaAllocation allocation)
	{
#ifdef DIA_DEBUG
		VmaAllocationInfo allocInfo;
		vmaGetAllocationInfo(s_Instance->m_Allocator, allocation, &allocInfo);
		s_Instance->m_TotalAllocatedBytes -= allocInfo.size;
#endif
		vmaDestroyBuffer(s_Instance->m_Allocator, buffer, allocation);
	}

	void Gfx_VulkanAllocator::UnmapMemory(VmaAllocation allocation)
	{
		vmaUnmapMemory(s_Instance->m_Allocator, allocation);
	}

	void Gfx_VulkanAllocator::GetAllocInfo(VmaAllocation allocation, VmaAllocationInfo& outInfo)
	{
		vmaGetAllocationInfo(s_Instance->m_Allocator, allocation, &outInfo);
	}

	VmaAllocator& Gfx_VulkanAllocator::GetAllocator()
	{
		return s_Instance->m_Allocator;
	}
}