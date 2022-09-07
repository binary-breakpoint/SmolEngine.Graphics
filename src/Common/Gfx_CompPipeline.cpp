#include "Gfx_Precompiled.h"
#include "Common/Gfx_CompPipeline.h"
#include "Common/Gfx_Descriptor.h"
#include "Common/Gfx_Shader.h"

namespace SmolEngine
{
	Gfx_CompPipeline::Gfx_CompPipeline()
		:
		m_Layout{nullptr},
		m_Pipeline{nullptr} {}

	Gfx_CompPipeline::~Gfx_CompPipeline()
	{
		Free();
	}

	void Gfx_CompPipeline::Create(CompPipelineCreateDesc* desc)
	{
		m_Desc = *desc;

		VkPipelineLayoutCreateInfo pipelineLayoutCI = {};
		pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCI.pNext = nullptr;
		pipelineLayoutCI.setLayoutCount = 1;
		pipelineLayoutCI.pSetLayouts = &desc->myDescriptor->m_Layout;

		if (m_Desc.myDescriptor->m_PushConstantRange.has_value())
		{
			pipelineLayoutCI.pushConstantRangeCount = 1;
			pipelineLayoutCI.pPushConstantRanges = &desc->myDescriptor->m_PushConstantRange.value();
		}

		VkDevice device = Gfx_Context::GetDevice().GetLogicalDevice();
		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &m_Layout));

		VkComputePipelineCreateInfo computePipelineCreateInfo{};
		computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		computePipelineCreateInfo.layout = m_Layout;
		computePipelineCreateInfo.stage = desc->myShader->m_ShaderStages[0];

		VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

		VK_CHECK_RESULT(vkCreateComputePipelines(device, nullptr, 1, &computePipelineCreateInfo, nullptr, &m_Pipeline));
	}

	void Gfx_CompPipeline::Free()
	{
		VK_DESTROY_DEVICE_HANDLE(m_Pipeline, vkDestroyPipeline);
		VK_DESTROY_DEVICE_HANDLE(m_Layout, vkDestroyPipelineLayout);
	}

	void Gfx_CompPipeline::CmdDispatch(Gfx_CmdBuffer* cmd, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ /*= 1*/)
	{
		vkCmdDispatch(cmd->GetBuffer(), groupCountX, groupCountY, groupCountZ);
	}

	void Gfx_CompPipeline::CmdPushConstant(Gfx_CmdBuffer* cmd, ShaderStage stage, uint32_t size, const void* data)
	{
		vkCmdPushConstants(cmd->GetBuffer(), m_Layout, Gfx_VulkanHelpers::GetShaderStage(stage), 0, size, data);
	}

	void Gfx_CompPipeline::CmdBindDescriptor(Gfx_CmdBuffer* cmd, Gfx_Descriptor* another /*= nullptr*/)
	{
		VkDescriptorSet set = another == nullptr ? m_Desc.myDescriptor->GetSet() : another->GetSet();
		vkCmdBindDescriptorSets(cmd->GetBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, m_Layout, 0, 1, &set, 0, 0);
	}

	void Gfx_CompPipeline::CmdBindPipeline(Gfx_CmdBuffer* cmd)
	{
		vkCmdBindPipeline(cmd->GetBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, m_Pipeline);
	}

	bool Gfx_CompPipeline::IsGood() const
	{
		return m_Desc.myShader != nullptr;
	}
}