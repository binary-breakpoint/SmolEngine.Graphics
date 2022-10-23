#include "Gfx_Precompiled.h"
#include "Common/Gfx_AccelStructure.h"
#include "Common/Gfx_Mesh.h"

#include "Backend/Gfx_VulkanHelpers.h"

#include <vulkan_memory_allocator/vk_mem_alloc.h>

namespace SmolEngine
{
	Gfx_AccelStructure::Gfx_AccelStructure()
		:
		m_Handle{nullptr},
		m_DstHandle{nullptr},
		m_NumBLASes{ 0 },
		m_DeviceAddress{ 0 } {}

	Gfx_AccelStructure::~Gfx_AccelStructure()
	{
		Free();
	}

	void Gfx_AccelStructure::Free()
	{
		const Gfx_VulkanDevice& device = Gfx_App::GetDevice();

		VK_DESTROY_DEVICE_HANDLE(m_Handle, device.vkDestroyAccelerationStructureKHR);
		VK_DESTROY_DEVICE_HANDLE(m_DstHandle, device.vkDestroyAccelerationStructureKHR);

		m_DeviceAddress = 0;
		m_NumBLASes = 0;
		m_Buffer.Free();
	}

	bool Gfx_AccelStructure::IsGood() const
	{
		return m_Handle != nullptr;
	}

	void Gfx_AccelStructure::BuildAsBottomLevel(uint32_t vertexStride, Gfx_Buffer* transform, Gfx_Mesh* mesh)
	{
		const Ref<Gfx_VertexBuffer>& vb = mesh->GetVertexBuffer();
		const Ref<Gfx_IndexBuffer>& ib = mesh->GetIndexBuffer();
		const Gfx_VulkanDevice& device = Gfx_App::GetDevice();

		VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
		vertexBufferDeviceAddress.deviceAddress = Gfx_VulkanHelpers::GetBufferDeviceAddress(vb->GetBuffer().GetRawBuffer());

		VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};
		indexBufferDeviceAddress.deviceAddress = Gfx_VulkanHelpers::GetBufferDeviceAddress(ib->GetBuffer().GetRawBuffer());

		VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress{};

		transformBufferDeviceAddress.deviceAddress = Gfx_VulkanHelpers::GetBufferDeviceAddress(transform->GetRawBuffer());

		// Build
		VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
		accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		accelerationStructureGeometry.flags = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;
		accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;

		VkAccelerationStructureGeometryTrianglesDataKHR triangles = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR };
		triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		triangles.vertexData = vertexBufferDeviceAddress;
		triangles.maxVertex = vb->GetVertexCount();
		triangles.vertexStride = vertexStride;
		triangles.indexType = VK_INDEX_TYPE_UINT32;
		triangles.indexData = indexBufferDeviceAddress;
		triangles.transformData.deviceAddress = 0;
		triangles.transformData.hostAddress = nullptr;
		triangles.transformData = transformBufferDeviceAddress;

		accelerationStructureGeometry.geometry.triangles = triangles;

		// Get size info
		VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
		accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR
			| VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
		accelerationStructureBuildGeometryInfo.geometryCount = 1;
		accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

		const uint32_t numTriangles = ib->GetCount() / 3;

		VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
		accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

		device.vkGetAccelerationStructureBuildSizesKHR(
			device.GetLogicalDevice(),
			VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&accelerationStructureBuildGeometryInfo,
			&numTriangles,
			&accelerationStructureBuildSizesInfo);

		BufferCreateDesc bufferDesc{};
		bufferDesc.myBufferUsage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		bufferDesc.myMemUsage = VMA_MEMORY_USAGE_GPU_ONLY;
		bufferDesc.mySize = accelerationStructureBuildSizesInfo.accelerationStructureSize;

		m_Buffer.Create(bufferDesc);

		VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
		accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		accelerationStructureCreateInfo.buffer = m_Buffer.GetRawBuffer();
		accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
		accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		device.vkCreateAccelerationStructureKHR(device.GetLogicalDevice(), &accelerationStructureCreateInfo, nullptr, &m_Handle);

		// Create a small scratch buffer used during build of the bottom level acceleration structure
		bufferDesc = {};
		bufferDesc.myFlags = BufferCreateDesc::CreateFlags::Scratch;
		bufferDesc.mySize = accelerationStructureBuildSizesInfo.buildScratchSize;
		Gfx_Buffer scratchBuffer{};
		scratchBuffer.Create(bufferDesc);

		VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
		accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
		accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		accelerationBuildGeometryInfo.dstAccelerationStructure = m_Handle;
		accelerationBuildGeometryInfo.geometryCount = 1;
		accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
		accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.GetDeviceAddress();

		VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
		accelerationStructureBuildRangeInfo.primitiveCount = numTriangles;
		accelerationStructureBuildRangeInfo.primitiveOffset = 0;
		accelerationStructureBuildRangeInfo.firstVertex = 0;
		accelerationStructureBuildRangeInfo.transformOffset = 0;

		std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

		// Build the acceleration structure on the device via a one-time command buffer submission
		// Some implementations may support acceleration structure building on the host (VkPhysicalDeviceAccelerationStructureFeaturesKHR->accelerationStructureHostCommands), but we prefer device builds

		Gfx_CmdBuffer cmdBuffer{};
		CmdBufferCreateDesc cmdDesc{};
		cmdBuffer.Create(&cmdDesc);

		cmdBuffer.CmdBeginRecord();
		{
			device.vkCmdBuildAccelerationStructuresKHR(
				cmdBuffer.GetBuffer(),
				1,
				&accelerationBuildGeometryInfo,
				accelerationBuildStructureRangeInfos.data());
		}
		cmdBuffer.CmdEndRecord();

		Gfx_VulkanHelpers::ExecuteCmdBuffer(&cmdBuffer);

		VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
		accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		accelerationDeviceAddressInfo.accelerationStructure = m_Handle;

		m_DeviceAddress = device.vkGetAccelerationStructureDeviceAddressKHR(device.GetLogicalDevice(), &accelerationDeviceAddressInfo);
	}

	void Gfx_AccelStructure::BuildAsTopLevel(Gfx_Buffer* instances, uint32_t primitiveCount, bool& out_update_descriptor)
	{
		const Gfx_VulkanDevice& device = Gfx_App::GetDevice();

		VkAccelerationStructureGeometryInstancesDataKHR instancesVk{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR };
		{
			instancesVk.data.deviceAddress = Gfx_VulkanHelpers::GetBufferDeviceAddress(instances->GetRawBuffer());
		}

		// Put the above into a VkAccelerationStructureGeometryKHR. We need to put the instances struct in a union and label it as instance data.
		VkAccelerationStructureGeometryKHR topASGeometry{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
		topASGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		topASGeometry.geometry.instances = instancesVk;

		bool update = false;
		if (m_Buffer.GetSize() > 0)
		{
			if (m_NumBLASes == primitiveCount)
				update = true;
			else
			{
				device.vkDestroyAccelerationStructureKHR(device.GetLogicalDevice(), m_Handle, nullptr);
				out_update_descriptor = true;
			}
		}

		// Find sizes
		VkAccelerationStructureBuildGeometryInfoKHR buildInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
		buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
		buildInfo.pGeometries = &topASGeometry;
		buildInfo.geometryCount = 1;
		buildInfo.mode = update ? VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR : VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;

		VkAccelerationStructureBuildSizesInfoKHR sizeInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
		device.vkGetAccelerationStructureBuildSizesKHR(device.GetLogicalDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo,
			&primitiveCount, &sizeInfo);

		const size_t bufferSize = m_Buffer.GetSize();
		if (sizeInfo.accelerationStructureSize > bufferSize)
		{
			if (bufferSize > 0)
			{
				m_Buffer.Free();
			}

			BufferCreateDesc desc{};
			desc.myBufferUsage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
			desc.mySize = sizeInfo.accelerationStructureSize;
			desc.myMemUsage = VMA_MEMORY_USAGE_GPU_ONLY;

			m_Buffer.Create(desc);
		}

		VkAccelerationStructureCreateInfoKHR acceleration_structure_create_info{};
		acceleration_structure_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		acceleration_structure_create_info.buffer = m_Buffer.GetRawBuffer();
		acceleration_structure_create_info.size = sizeInfo.accelerationStructureSize;
		acceleration_structure_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

		device.vkCreateAccelerationStructureKHR(device.GetLogicalDevice(), &acceleration_structure_create_info, nullptr,
			update ? &m_DstHandle : &m_Handle);

		BufferCreateDesc bufferDesc = {};
		bufferDesc.myFlags = BufferCreateDesc::CreateFlags::Scratch;
		bufferDesc.mySize = sizeInfo.buildScratchSize;
		Gfx_Buffer scratchBuffer{};
		scratchBuffer.Create(bufferDesc);

		// Update build information
		buildInfo.srcAccelerationStructure = update ? m_Handle : VK_NULL_HANDLE;
		buildInfo.dstAccelerationStructure = update ? m_DstHandle : m_Handle;
		buildInfo.scratchData.deviceAddress = scratchBuffer.GetDeviceAddress();

		VkAccelerationStructureBuildRangeInfoKHR acceleration_structure_build_range_info;
		acceleration_structure_build_range_info.primitiveCount = primitiveCount;
		acceleration_structure_build_range_info.primitiveOffset = 0;
		acceleration_structure_build_range_info.firstVertex = 0;
		acceleration_structure_build_range_info.transformOffset = 0;
		std::vector<VkAccelerationStructureBuildRangeInfoKHR*> acceleration_build_structure_range_infos = { &acceleration_structure_build_range_info };

		Gfx_CmdBuffer cmdBuffer{};
		CmdBufferCreateDesc cmdDesc{};
		cmdBuffer.Create(&cmdDesc);

		cmdBuffer.CmdBeginRecord();
		{
			device.vkCmdBuildAccelerationStructuresKHR(
				cmdBuffer.GetBuffer(),
				1,
				&buildInfo,
				acceleration_build_structure_range_infos.data());
		}
		cmdBuffer.CmdEndRecord();

		Gfx_VulkanHelpers::ExecuteCmdBuffer(&cmdBuffer);

		// Get the top acceleration structure's handle, which will be used to set up its descriptor
		VkAccelerationStructureDeviceAddressInfoKHR acceleration_device_address_info{};
		acceleration_device_address_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		acceleration_device_address_info.accelerationStructure = update ? m_DstHandle : m_Handle;

		m_NumBLASes = primitiveCount;
		m_DeviceAddress = device.vkGetAccelerationStructureDeviceAddressKHR(device.GetLogicalDevice(), &acceleration_device_address_info);
	}

	VkAccelerationStructureKHR Gfx_AccelStructure::GetHandle()
	{
		return m_Handle;
	}

	Gfx_Buffer& Gfx_AccelStructure::GetBuffer()
	{
		return m_Buffer;
	}

	uint64_t Gfx_AccelStructure::GetDeviceAddress() const
	{
		return m_DeviceAddress;
	}

}