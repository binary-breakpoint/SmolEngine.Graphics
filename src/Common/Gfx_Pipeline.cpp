#include "Gfx_Precompiled.h"
#include "Common/Gfx_Pipeline.h"
#include "Common/Gfx_Framebuffer.h"
#include "Common/Gfx_Descriptor.h"
#include "Common/Gfx_Shader.h"
#include "Common/Gfx_Mesh.h"
#include "Common/Gfx_CmdBuffer.h"

#include "Backend/Gfx_VulkanHelpers.h"

namespace SmolEngine
{
	Gfx_Pipeline::Gfx_Pipeline(Type type)
		: m_Layout{ nullptr }
		, m_Pipeline{ nullptr }
		, m_Type{type}
	{

	}

	Gfx_Pipeline::~Gfx_Pipeline()
	{
		Free();
	}

	void Gfx_Pipeline::Free()
	{
		VK_DESTROY_DEVICE_HANDLE(m_Layout, vkDestroyPipelineLayout);
		VK_DESTROY_DEVICE_HANDLE(m_Pipeline, vkDestroyPipeline);
	}

	Gfx_Pipeline::Type Gfx_Pipeline::GetType() const
	{
		return m_Type;
	}

	bool Gfx_Pipeline::IsType(Type type) const
	{
		return m_Type == type;
	}

	VkPipelineLayout Gfx_Pipeline::GetLayout() const
	{
		return m_Layout;
	}

	VkPipeline Gfx_Pipeline::GetPipeline() const
	{
		return m_Pipeline;
	}

	static bool locIsBlendEnabled(const GraphicsPipelineCreateDesc* desc)
	{
		return desc->mySrcColorBlendFactor != BlendFactor::NONE || desc->myDstColorBlendFactor != BlendFactor::NONE ||
			desc->myDstAlphaBlendFactor != BlendFactor::NONE || desc->mySrcAlphaBlendFactor != BlendFactor::NONE;
	}

	static bool locIsPipelineCreateDescValid(const GraphicsPipelineCreateDesc* desc)
	{
		return desc->myShader != nullptr || desc->myDescriptor != nullptr || desc->myFramebuffer != nullptr;
	}

	void Gfx_GraphicsPipeline::Create(GraphicsPipelineCreateDesc* desc)
	{
		GFX_ASSERT(locIsPipelineCreateDescValid(desc))

		m_Desc = *desc;
		VkDevice device = Gfx_App::GetDevice().GetLogicalDevice();

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

		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &m_Layout));

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = Gfx_VulkanHelpers::GetTopology(desc->myDrawMode);
		inputAssemblyState.primitiveRestartEnable = desc->myPrimitiveRestartEnable;

		VkPipelineRasterizationStateCreateInfo rasterizationState = {};
		rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationState.polygonMode = Gfx_VulkanHelpers::GetPolygonMode(desc->myPolygonMode);
		rasterizationState.cullMode = Gfx_VulkanHelpers::GetCullMode(desc->myCullMode);
		rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationState.depthClampEnable = Gfx_App::GetDevice().GetDeviceFeatures()->depthClamp;
		rasterizationState.rasterizerDiscardEnable = VK_FALSE;
		rasterizationState.depthBiasEnable = desc->myDepthBiasEnabled;
		rasterizationState.lineWidth = 1.0f;

		std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentState;
		{
			uint32_t count = static_cast<uint32_t>(m_Desc.myFramebuffer->GetDesc().myAttachments.size());
			count = m_Desc.myFramebuffer->GetDepthAttachment() != nullptr ? count - 1 : count;
			blendAttachmentState.resize(count);

			for (uint32_t i = 0; i < count; ++i)
			{
				blendAttachmentState[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
				blendAttachmentState[i].blendEnable = locIsBlendEnabled(desc) ? VK_TRUE : VK_FALSE;
				blendAttachmentState[i].srcColorBlendFactor = Gfx_VulkanHelpers::GetBlendFactor(desc->mySrcColorBlendFactor);
				blendAttachmentState[i].dstColorBlendFactor = Gfx_VulkanHelpers::GetBlendFactor(desc->myDstColorBlendFactor);;
				blendAttachmentState[i].colorBlendOp = Gfx_VulkanHelpers::GetBlendOp(desc->myColorBlendOp);
				blendAttachmentState[i].srcAlphaBlendFactor = Gfx_VulkanHelpers::GetBlendFactor(desc->mySrcAlphaBlendFactor);
				blendAttachmentState[i].dstAlphaBlendFactor = Gfx_VulkanHelpers::GetBlendFactor(desc->myDstAlphaBlendFactor);
				blendAttachmentState[i].alphaBlendOp = Gfx_VulkanHelpers::GetBlendOp(desc->myAlphaBlendOp);
			}
		}

		VkPipelineColorBlendStateCreateInfo colorBlendState = {};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentState.size());
		colorBlendState.pAttachments = blendAttachmentState.data();

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		std::vector<VkDynamicState> dynamicStateEnables;
		{
			dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
			dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);

			if (desc->myDepthBiasEnabled)
				dynamicStateEnables.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);


			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.pDynamicStates = dynamicStateEnables.data();
			dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
		}

		VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = desc->myDepthTestEnabled;
		depthStencilState.depthWriteEnable = desc->myDepthWriteEnabled;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
		depthStencilState.stencilTestEnable = VK_FALSE;
		depthStencilState.minDepthBounds = desc->myMinDepth;
		depthStencilState.maxDepthBounds = desc->myMaxDepth;

		VkPipelineMultisampleStateCreateInfo multisampleState = {};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		std::vector<VkVertexInputBindingDescription> vertexInputBindings(desc->myVertexInput.size());
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributs;
		{
			uint32_t index = 0;
			uint32_t location = 0;
			for (const auto& input : desc->myVertexInput)
			{
				vertexInputBindings[index].binding = index;
				vertexInputBindings[index].stride = input.GetStride();
				vertexInputBindings[index].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

				for (const auto& element : input.GetElements())
				{
					VkVertexInputAttributeDescription inputAttributeDescription;
					{
						inputAttributeDescription.binding = index;
						inputAttributeDescription.location = location;
						inputAttributeDescription.format = Gfx_VulkanHelpers::GetFormat(element.m_Format);
						inputAttributeDescription.offset = element.m_Offset;
					}

					vertexInputAttributs.emplace_back(inputAttributeDescription);
					location++;
				}

				index++;
			}
		}

		VkPipelineVertexInputStateCreateInfo vertexInputState = {};
		vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		if (desc->myVertexInput.size() > 0)
		{
			vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
			vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
			vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributs.size());
			vertexInputState.pVertexAttributeDescriptions = vertexInputAttributs.data();
		}

		VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.layout = m_Layout;
		pipelineCreateInfo.renderPass = desc->myFramebuffer->GetRenderPass();
		pipelineCreateInfo.pVertexInputState = &vertexInputState;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pDynamicState = &dynamicState;

		auto& shaderStages = desc->myShader->GetShaderStages();
		pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineCreateInfo.pStages = shaderStages.data();

		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineCreateInfo, nullptr, &m_Pipeline));
	}

	void Gfx_GraphicsPipeline::Reload()
	{
		
	}

	bool Gfx_GraphicsPipeline::IsGood() const
	{
		return m_Desc.myShader != nullptr;
	}

	void Gfx_ComputePipeline::Create(ComputePipelineCreateDesc* desc)
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

		VkDevice device = Gfx_App::GetDevice().GetLogicalDevice();
		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &m_Layout));

		VkComputePipelineCreateInfo computePipelineCreateInfo{};
		computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		computePipelineCreateInfo.layout = m_Layout;
		computePipelineCreateInfo.stage = desc->myShader->m_ShaderStages[0];

		VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

		VK_CHECK_RESULT(vkCreateComputePipelines(device, nullptr, 1, &computePipelineCreateInfo, nullptr, &m_Pipeline));
	}

	bool Gfx_ComputePipeline::IsGood() const
	{
		return m_Desc.myShader != nullptr;
	}

	void Gfx_RaytracingPipeline::Create(RaytracingPipelineCreateDesc* desc)
	{
		GFX_ASSERT(desc->myDescriptor || desc->myShader)

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

		VkDevice device = Gfx_App::GetDevice().GetLogicalDevice();
		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &m_Layout));

		VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCI = {};
		rayTracingPipelineCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		rayTracingPipelineCI.stageCount = static_cast<uint32_t>(desc->myShader->m_ShaderStages.size());
		rayTracingPipelineCI.pStages = desc->myShader->m_ShaderStages.data();
		rayTracingPipelineCI.groupCount = static_cast<uint32_t>(desc->myShader->m_ShaderGroupsRT.size());
		rayTracingPipelineCI.pGroups = desc->myShader->m_ShaderGroupsRT.data();
		rayTracingPipelineCI.maxPipelineRayRecursionDepth = desc->myMaxRayRecursionDepth;
		rayTracingPipelineCI.layout = m_Layout;

		VK_CHECK_RESULT(Gfx_App::GetDevice().vkCreateRayTracingPipelinesKHR(device, VK_NULL_HANDLE,
			VK_NULL_HANDLE, 1, &rayTracingPipelineCI, nullptr, &m_Pipeline));

		m_Desc.myShader->CreateBindingTable(m_Pipeline);
	}

	bool Gfx_RaytracingPipeline::IsGood() const
	{
		return m_Desc.myShader != nullptr;
	}
}