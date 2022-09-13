#pragma once
#include "Backend/Gfx_VulkanCore.h"

#include <vector>

namespace SmolEngine
{
	class Gfx_VulkanInstance;

	class Gfx_VulkanDevice
	{
		friend class VulkanCommandPool;
		friend class DenoisePass;

	public:
		struct QueueFamilyIndices
		{
			int32_t Graphics = -1;
			int32_t Compute = -1;
			int32_t Transfer = -1;
		};

		enum class QueueFamilyFlags
		{
			Graphics,
			Compute,
			Transfer
		};

		Gfx_VulkanDevice();

		bool Init(const Gfx_VulkanInstance* instance);			

		uint32_t GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags memFlags) const;
		const VkPhysicalDeviceMemoryProperties* GetMemoryProperties() const;
		const VkPhysicalDeviceProperties* GetDeviceProperties() const;
		const VkPhysicalDeviceFeatures* GetDeviceFeatures() const;
		const VkPhysicalDevice GetPhysicalDevice() const;
		const VkDevice GetLogicalDevice() const;
		const QueueFamilyIndices& GetQueueFamilyIndices() const;
		const VkQueue GetQueue(QueueFamilyFlags flag) const;
		bool GetRaytracingSupport() const;

		PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR;
		PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
		PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;
		PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
		PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
		PFN_vkBuildAccelerationStructuresKHR vkBuildAccelerationStructuresKHR;
		PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
		PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;
		PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;
		PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;

		VkPhysicalDeviceRayTracingPipelinePropertiesKHR  rayTracingPipelineProperties{};
		VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};

	private:											 
		bool SetupPhysicalDevice(const Gfx_VulkanInstance* instance);
		bool HasRequiredExtensions(const VkPhysicalDevice& device, const std::vector<const char*>& extensionsList);
		QueueFamilyIndices GetQueueFamilyIndices(int flags);
		bool SetupLogicalDevice();

		void SelectDevice(VkPhysicalDevice device);
		void GetFuncPtrs();

		VkQueue m_GraphicsQueue;
		VkQueue m_ComputeQueue;
		VkCommandPool m_CommandPool;
		VkCommandPool m_ComputeCommandPool;
		VkPhysicalDevice m_PhysicalDevice;
		VkDevice m_LogicalDevice;
		VkPhysicalDeviceProperties m_DeviceProperties;
		VkPhysicalDeviceFeatures m_DeviceFeatures;
		VkPhysicalDeviceMemoryProperties m_MemoryProperties;
		QueueFamilyIndices m_QueueFamilyIndices;
		std::vector<VkQueueFamilyProperties>  m_QueueFamilyProperties;
		std::vector<const char*> m_ExtensionsList;
		bool m_RayTracingEnabled;
	};
}