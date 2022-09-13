#include "Gfx_Precompiled.h"
#include "Backend/Gfx_VulkanDevice.h"
#include "Backend/Gfx_VulkanInstance.h"

#include <vulkan/vulkan_win32.h>

namespace SmolEngine
{
	Gfx_VulkanDevice::Gfx_VulkanDevice() :
		m_GraphicsQueue{ nullptr },
		m_ComputeQueue{ nullptr },
		m_CommandPool{ nullptr },
		m_ComputeCommandPool{nullptr},
		m_PhysicalDevice{nullptr},
		m_LogicalDevice{nullptr},
		m_RayTracingEnabled{false}
	{

	}

	bool Gfx_VulkanDevice::Init(const Gfx_VulkanInstance* instance)
	{
		if (SetupPhysicalDevice(instance))
		{
			std::stringstream ss;
			ss << "Vulkan Info:\n\n";
			ss << "           Vulkan API Version: " << std::to_string(m_DeviceProperties.apiVersion) << "\n";
			ss << "           Selected Device: " << std::string(m_DeviceProperties.deviceName) << "\n";
			ss << "           Driver Version: " << std::to_string(m_DeviceProperties.driverVersion) << "\n";
			ss << "           Raytracing Enabled: " << std::to_string(m_RayTracingEnabled) << "\n";
			ss << "           Max push_constant size: " << std::to_string(m_DeviceProperties.limits.maxPushConstantsSize) << "\n\n";

			GFX_LOG(ss.str(), Gfx_Log::Level::Info)

			return SetupLogicalDevice();
		}

		return false;
	}

	bool Gfx_VulkanDevice::SetupPhysicalDevice(const Gfx_VulkanInstance* _instance)
	{
		const VkInstance& instance = _instance->GetInstance();
	 
		m_ExtensionsList.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		m_ExtensionsList.push_back(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);
		m_ExtensionsList.push_back(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);

#ifdef AFTERMATH
		m_ExtensionsList.push_back(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME);
		m_ExtensionsList.push_back(VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME);
#endif
		std::vector<const char*> rayTracingEX = m_ExtensionsList;

		// Ray tracing related extensions
		rayTracingEX.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
		rayTracingEX.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
		// Required by VK_KHR_acceleration_structure
		rayTracingEX.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
		rayTracingEX.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
		// Required for VK_KHR_ray_tracing_pipeline
		rayTracingEX.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
		// Required by VK_KHR_spirv_1_4
		rayTracingEX.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);

		rayTracingEX.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);

		rayTracingEX.push_back("VK_NVX_binary_import");
		rayTracingEX.push_back("VK_NVX_image_view_handle");

		uint32_t devicesCount = 0;
		vkEnumeratePhysicalDevices(instance, &devicesCount, nullptr);
		std::unique_ptr<VkPhysicalDevice[]> devices = std::make_unique<VkPhysicalDevice[]>(devicesCount);
		vkEnumeratePhysicalDevices(instance, &devicesCount, devices.get());

		for (uint32_t i = 0; i < devicesCount; ++i)
		{
			const VkPhysicalDevice& current_device = devices[i];

			if (HasRequiredExtensions(current_device, rayTracingEX)) // Ray Tracing
			{
				m_ExtensionsList = rayTracingEX;
				m_RayTracingEnabled = true;
			
				SelectDevice(current_device);
				break;
			}

			if (HasRequiredExtensions(current_device, m_ExtensionsList)) // No Ray Tracing
				SelectDevice(current_device);
		}

		return m_PhysicalDevice != VK_NULL_HANDLE;
	}

	bool Gfx_VulkanDevice::SetupLogicalDevice()
	{
		const float priority = 0.0f;
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

		VkDeviceQueueCreateInfo queueCreateInfo = {};
		{
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = m_QueueFamilyIndices.Graphics;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &priority;

			queueCreateInfos.push_back(queueCreateInfo);
		}

		// If compute family index differs, we need an additional queue create info for the compute queue
		if (m_QueueFamilyIndices.Compute != m_QueueFamilyIndices.Graphics)
		{
			queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = m_QueueFamilyIndices.Compute;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &priority;

			queueCreateInfos.push_back(queueCreateInfo);
		}

		if ((m_QueueFamilyIndices.Transfer != m_QueueFamilyIndices.Graphics) && (m_QueueFamilyIndices.Transfer != m_QueueFamilyIndices.Compute))
		{
			queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = m_QueueFamilyIndices.Transfer;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &priority;

			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures = {};
		descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		descriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
		descriptorIndexingFeatures.runtimeDescriptorArray = VK_TRUE;


		VkPhysicalDevice16BitStorageFeatures enabled16BitStorageFeatures = {};
		enabled16BitStorageFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES;
		enabled16BitStorageFeatures.storageBuffer16BitAccess = VK_TRUE;
		enabled16BitStorageFeatures.pNext = &descriptorIndexingFeatures;

		VkPhysicalDeviceFloat16Int8FeaturesKHR enabledFloat16Int8FeaturesKHR = {};
		enabledFloat16Int8FeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT16_INT8_FEATURES_KHR;
		enabledFloat16Int8FeaturesKHR.shaderFloat16 = VK_TRUE;
		enabledFloat16Int8FeaturesKHR.pNext = &enabled16BitStorageFeatures;

		VkPhysicalDeviceTimelineSemaphoreFeatures enabledTimelineSemaphoreFeatures = {};
		enabledTimelineSemaphoreFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
		enabledTimelineSemaphoreFeatures.timelineSemaphore = VK_TRUE;
		enabledTimelineSemaphoreFeatures.pNext = &enabledFloat16Int8FeaturesKHR;

		// Enable features required for ray tracing using feature chaining via pNext	
		VkPhysicalDeviceBufferDeviceAddressFeatures enabledBufferDeviceAddresFeatures{};
		enabledBufferDeviceAddresFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
		enabledBufferDeviceAddresFeatures.bufferDeviceAddress = VK_TRUE;
		enabledBufferDeviceAddresFeatures.pNext = &enabledTimelineSemaphoreFeatures;

		VkPhysicalDeviceRayTracingPipelineFeaturesKHR enabledRayTracingPipelineFeatures{};
		enabledRayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
		enabledRayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
		enabledRayTracingPipelineFeatures.pNext = &enabledBufferDeviceAddresFeatures;

		VkPhysicalDeviceAccelerationStructureFeaturesKHR enabledAccelerationStructureFeatures{};
		enabledAccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
		enabledAccelerationStructureFeatures.accelerationStructure = VK_TRUE;
		enabledAccelerationStructureFeatures.pNext = &enabledRayTracingPipelineFeatures;

		VkDeviceDiagnosticsConfigCreateInfoNV diagnosticsConfigCreateInfoNV{};
		diagnosticsConfigCreateInfoNV.sType = VK_STRUCTURE_TYPE_DEVICE_DIAGNOSTICS_CONFIG_CREATE_INFO_NV;
		diagnosticsConfigCreateInfoNV.flags = VK_DEVICE_DIAGNOSTICS_CONFIG_ENABLE_SHADER_DEBUG_INFO_BIT_NV | VK_DEVICE_DIAGNOSTICS_CONFIG_ENABLE_RESOURCE_TRACKING_BIT_NV
			| VK_DEVICE_DIAGNOSTICS_CONFIG_ENABLE_AUTOMATIC_CHECKPOINTS_BIT_NV;
		diagnosticsConfigCreateInfoNV.pNext = m_RayTracingEnabled ? &enabledAccelerationStructureFeatures : nullptr;


		VkDeviceCreateInfo deviceInfo = {};
		{
			deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
			deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
			deviceInfo.ppEnabledExtensionNames = m_ExtensionsList.data();
			deviceInfo.enabledExtensionCount = static_cast<uint32_t>(m_ExtensionsList.size());
			deviceInfo.pEnabledFeatures = &m_DeviceFeatures;

#ifdef AFTERMATH
			deviceInfo.pNext = &diagnosticsConfigCreateInfoNV;
#else
			if (m_RayTracingEnabled)
			{
				deviceInfo.pNext = &enabledAccelerationStructureFeatures;
			}
#endif
		}

		VkResult result = vkCreateDevice(m_PhysicalDevice, &deviceInfo, nullptr, &m_LogicalDevice);

		vkGetDeviceQueue(m_LogicalDevice, m_QueueFamilyIndices.Graphics, 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_LogicalDevice, m_QueueFamilyIndices.Compute, 0, &m_ComputeQueue);

		GetFuncPtrs();

		assert(result == VK_SUCCESS);
		return result == VK_SUCCESS;
	}

	bool Gfx_VulkanDevice::HasRequiredExtensions(const VkPhysicalDevice& device, const std::vector<const char*>& extensionsList)
	{
		uint32_t extCount = 0;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, nullptr);
		std::unique_ptr<VkExtensionProperties[]> extensions = std::make_unique<VkExtensionProperties[]>(extCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, extensions.get());
		for (const auto& name : extensionsList)
		{
			bool ext_found = false;

			for (uint32_t i = 0; i < extCount; ++i)
			{
				// Note:
				// The return value from strcmp is 0 if the two strings are equal

				if (strcmp(extensions[i].extensionName, name) == 0)
				{
					ext_found = true;
					break;
				}
			}

			if (ext_found == false)
			{
				return false;
			}
		}

		return true;
	}

	void Gfx_VulkanDevice::SelectDevice(VkPhysicalDevice device)
	{
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		assert(queueFamilyCount > 0);

		m_QueueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, m_QueueFamilyProperties.data());

		VkPhysicalDeviceProperties2 deviceProperties2{};
		deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		// Get ray tracing pipeline properties
		if (m_RayTracingEnabled)
		{
			rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
			deviceProperties2.pNext = &rayTracingPipelineProperties;
		}
		vkGetPhysicalDeviceProperties2(device, &deviceProperties2);

		VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
		deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		// Get acceleration structure properties
		if (m_RayTracingEnabled)
		{
			accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
			deviceFeatures2.pNext = &accelerationStructureFeatures;
		}
		vkGetPhysicalDeviceFeatures2(device, &deviceFeatures2);

		VkPhysicalDeviceMemoryProperties memoryProperties = {};
		vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);

		if (deviceProperties2.properties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			int requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;

			m_QueueFamilyIndices = GetQueueFamilyIndices(requestedQueueTypes);
			m_DeviceProperties = deviceProperties2.properties;
			m_DeviceFeatures = deviceFeatures2.features;
			m_MemoryProperties = memoryProperties;
			m_DeviceFeatures.shaderSampledImageArrayDynamicIndexing = VK_TRUE;
			m_PhysicalDevice = device;
		}
	}

	void Gfx_VulkanDevice::GetFuncPtrs()
	{
		if (m_RayTracingEnabled)
		{
			// Get the function pointers required for ray tracing
			vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(m_LogicalDevice, "vkGetBufferDeviceAddressKHR"));
			vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(m_LogicalDevice, "vkCmdBuildAccelerationStructuresKHR"));
			vkBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(m_LogicalDevice, "vkBuildAccelerationStructuresKHR"));
			vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(m_LogicalDevice, "vkCreateAccelerationStructureKHR"));
			vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(m_LogicalDevice, "vkDestroyAccelerationStructureKHR"));
			vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(m_LogicalDevice, "vkGetAccelerationStructureBuildSizesKHR"));
			vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(m_LogicalDevice, "vkGetAccelerationStructureDeviceAddressKHR"));
			vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(m_LogicalDevice, "vkCmdTraceRaysKHR"));
			vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(m_LogicalDevice, "vkGetRayTracingShaderGroupHandlesKHR"));
			vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(m_LogicalDevice, "vkCreateRayTracingPipelinesKHR"));
		}
	}

	Gfx_VulkanDevice::QueueFamilyIndices Gfx_VulkanDevice::GetQueueFamilyIndices(int flags)
	{
		QueueFamilyIndices indices;

		// Dedicated queue for compute
		// Try to find a queue family index that supports compute but not graphics
		if (flags & VK_QUEUE_COMPUTE_BIT)
		{
			for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); i++)
			{
				auto& queueFamilyProperties = m_QueueFamilyProperties[i];
				if ((queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT) && ((queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
				{
					indices.Compute = i;
					break;
				}
			}
		}

		// Dedicated queue for transfer
		// Try to find a queue family index that supports transfer but not graphics and compute
		if (flags & VK_QUEUE_TRANSFER_BIT)
		{
			for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); i++)
			{
				auto& queueFamilyProperties = m_QueueFamilyProperties[i];
				if ((queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT) && ((queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
				{
					indices.Transfer = i;
					break;
				}
			}
		}

		// For other queue types or if no separate compute queue is present, return the first one to support the requested flags
		for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); i++)
		{
			if ((flags & VK_QUEUE_TRANSFER_BIT) && indices.Transfer == -1)
			{
				if (m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
					indices.Transfer = i;
			}

			if ((flags & VK_QUEUE_COMPUTE_BIT) && indices.Compute == -1)
			{
				if (m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
					indices.Compute = i;
			}

			if (flags & VK_QUEUE_GRAPHICS_BIT)
			{
				if (m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
					indices.Graphics = i;
			}
		}

		return indices;
	}

	const VkPhysicalDeviceMemoryProperties* Gfx_VulkanDevice::GetMemoryProperties() const
	{
		return &m_MemoryProperties;
	}

	const VkPhysicalDeviceProperties* Gfx_VulkanDevice::GetDeviceProperties() const
	{
		return &m_DeviceProperties;
	}

	const VkPhysicalDeviceFeatures* Gfx_VulkanDevice::GetDeviceFeatures() const
	{
		return &m_DeviceFeatures;
	}

	const VkPhysicalDevice Gfx_VulkanDevice::GetPhysicalDevice() const
	{
		return m_PhysicalDevice;
	}

	const VkDevice Gfx_VulkanDevice::GetLogicalDevice() const
	{
		return m_LogicalDevice;
	}

	const Gfx_VulkanDevice::QueueFamilyIndices& Gfx_VulkanDevice::GetQueueFamilyIndices() const
	{
		return m_QueueFamilyIndices;
	}

	uint32_t Gfx_VulkanDevice::GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags memFlags) const
	{
		for (uint32_t i = 0; i < m_MemoryProperties.memoryTypeCount; ++i)
		{
			if ((typeBits & 1) == 1)
			{
				if ((m_MemoryProperties.memoryTypes[i].propertyFlags & memFlags) == memFlags)
				{
					return i;
				}
			}

			typeBits >>= 1;
		}

		GFX_ASSERT(true, "VulkanDevice: Could not find a suitable memory type!")
		return 0;
	}

	const VkQueue Gfx_VulkanDevice::GetQueue(Gfx_VulkanDevice::QueueFamilyFlags flag) const
	{
		return flag == QueueFamilyFlags::Compute ? m_ComputeQueue : m_GraphicsQueue;
	}

	bool Gfx_VulkanDevice::GetRaytracingSupport() const
	{
		return m_RayTracingEnabled;
	}
}