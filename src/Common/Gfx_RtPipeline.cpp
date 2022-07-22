#include "Gfx_Precompiled.h"
#include "Common/Gfx_RtPipeline.h"
#include "Common/Gfx_Shader.h"
#include "Common/Gfx_CmdBuffer.h"
#include "Common/Gfx_Descriptor.h"
#include "Backend/Gfx_VulkanHelpers.h"

namespace Dia
{
	Gfx_RtPipeline::Gfx_RtPipeline()
		:
		m_Pipeline{nullptr},
		m_Layout{nullptr} {}

	Gfx_RtPipeline::~Gfx_RtPipeline()
	{
		Free();
	}

	void Gfx_RtPipeline::Create(RtPipelineCreateDesc* desc)
	{
		if (!desc->myDescriptor  || !desc->myShader)
		{
			// AASERT
		}

		m_Desc = *desc;

		VkPipelineLayoutCreateInfo pipelineLayoutCI = {};
		pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCI.pNext = nullptr;
		pipelineLayoutCI.setLayoutCount = 1;
		pipelineLayoutCI.pSetLayouts = &desc->myDescriptor->m_Layout;

		if (desc->myDescriptor->m_PushConstantRange.has_value())
		{
			pipelineLayoutCI.pushConstantRangeCount = 1;
			pipelineLayoutCI.pPushConstantRanges = &desc->myDescriptor->m_PushConstantRange.value();
		}

		VkDevice device = Gfx_Context::GetDevice().GetLogicalDevice();
		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &m_Layout));

		VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCI = {};
		rayTracingPipelineCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		rayTracingPipelineCI.stageCount = static_cast<uint32_t>(desc->myShader->m_ShaderStages.size());
		rayTracingPipelineCI.pStages = desc->myShader->m_ShaderStages.data();
		rayTracingPipelineCI.groupCount = static_cast<uint32_t>(desc->myShader->m_ShaderGroupsRT.size());
		rayTracingPipelineCI.pGroups = desc->myShader->m_ShaderGroupsRT.data();
		rayTracingPipelineCI.maxPipelineRayRecursionDepth = desc->myMaxRayRecursionDepth;
		rayTracingPipelineCI.layout = m_Layout;

		VK_CHECK_RESULT(Gfx_Context::GetDevice().vkCreateRayTracingPipelinesKHR(device, VK_NULL_HANDLE,
			VK_NULL_HANDLE, 1, &rayTracingPipelineCI, nullptr, &m_Pipeline));

		m_Desc.myShader->CreateBindingTable(m_Pipeline);
	}

	void Gfx_RtPipeline::CmdDispatch(Gfx_CmdBuffer* cmd, DispatchDesc* desc)
	{
		std::unordered_map<ShaderStage, Gfx_Buffer>& bindingTable = m_Desc.myShader->m_BindingTables;
		const Gfx_VulkanDevice& device = Gfx_Context::GetDevice();

		const uint32_t handleSizeAligned = Gfx_VulkanHelpers::GetAlignedSize(device.rayTracingPipelineProperties.shaderGroupHandleSize,
			device.rayTracingPipelineProperties.shaderGroupHandleAlignment);

		VkStridedDeviceAddressRegionKHR raygenShaderSbtEntry{};
		raygenShaderSbtEntry.deviceAddress = Gfx_VulkanHelpers::GetBufferDeviceAddress(bindingTable[ShaderStage::RayGen].GetRawBuffer());
		raygenShaderSbtEntry.stride = handleSizeAligned;
		raygenShaderSbtEntry.size = handleSizeAligned;

		VkStridedDeviceAddressRegionKHR missShaderSbtEntry{};
		missShaderSbtEntry.deviceAddress = Gfx_VulkanHelpers::GetBufferDeviceAddress(bindingTable[desc->myRayMiss].GetRawBuffer());
		missShaderSbtEntry.stride = handleSizeAligned;
		missShaderSbtEntry.size = handleSizeAligned;

		VkStridedDeviceAddressRegionKHR hitShaderSbtEntry{};
		hitShaderSbtEntry.deviceAddress = Gfx_VulkanHelpers::GetBufferDeviceAddress(bindingTable[desc->myRayHit].GetRawBuffer());
		hitShaderSbtEntry.stride = handleSizeAligned;
		hitShaderSbtEntry.size = handleSizeAligned;

		VkStridedDeviceAddressRegionKHR callableShaderSbtEntry{};
		if(desc->myAnyHitOrCallable.has_value())
		{
			callableShaderSbtEntry.deviceAddress = Gfx_VulkanHelpers::GetBufferDeviceAddress(
				bindingTable[desc->myAnyHitOrCallable.value()].GetRawBuffer());
			callableShaderSbtEntry.stride = handleSizeAligned;
			callableShaderSbtEntry.size = handleSizeAligned;
		}

		device.vkCmdTraceRaysKHR(
			cmd->GetBuffer(),
			&raygenShaderSbtEntry,
			&missShaderSbtEntry,
			&hitShaderSbtEntry,
			&callableShaderSbtEntry,
			desc->myWidth,
			desc->myHeight,
			1);
	}

	void Gfx_RtPipeline::CmdPushConstant(Gfx_CmdBuffer* cmd, ShaderStage stage, uint32_t size, const void* data)
	{
		vkCmdPushConstants(cmd->GetBuffer(), m_Layout, Gfx_VulkanHelpers::GetShaderStage(stage), 0, size, data);
	}

	void Gfx_RtPipeline::CmdBindDescriptor(Gfx_CmdBuffer* cmd, Gfx_Descriptor* another /*= nullptr*/)
	{
		VkDescriptorSet set = another == nullptr ? m_Desc.myDescriptor->GetSet() : another->GetSet();
		vkCmdBindDescriptorSets(cmd->GetBuffer(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
			m_Layout, 0, 1, &set, 0, 0);
	}

	void Gfx_RtPipeline::CmdBindPipeline(Gfx_CmdBuffer* cmd)
	{
		vkCmdBindPipeline(cmd->GetBuffer(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_Pipeline);
	}

	void Gfx_RtPipeline::Free()
	{
		VK_DESTROY_DEVICE_HANDLE(m_Pipeline, vkDestroyPipeline);
		VK_DESTROY_DEVICE_HANDLE(m_Layout, vkDestroyPipelineLayout);
	}

	bool Gfx_RtPipeline::IsGood() const
	{
		return m_Desc.myShader != nullptr;
	}
}