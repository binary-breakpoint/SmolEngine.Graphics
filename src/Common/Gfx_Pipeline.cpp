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
	static bool locIsBlendEnableEnabled(const PipelineCreateDesc* desc)
	{
		return desc->mySrcColorBlendFactor != BlendFactor::NONE || desc->myDstColorBlendFactor != BlendFactor::NONE ||
			desc->myDstAlphaBlendFactor != BlendFactor::NONE || desc->mySrcAlphaBlendFactor != BlendFactor::NONE;
	}

	static bool locIsPipelineCreateDescValid(const PipelineCreateDesc* desc)
	{
		return desc->myShader != nullptr || desc->myDescriptor != nullptr || desc->myFramebuffer != nullptr;
	}

	Gfx_Pipeline::Gfx_Pipeline()
		:
		m_Layout{nullptr},
		m_DrawMode{0} {}

	Gfx_Pipeline::~Gfx_Pipeline()
	{
		Free();
	}

	void Gfx_Pipeline::CmdBeginRenderPass(Gfx_CmdBuffer* cmd)
	{
		uint32_t width = m_Desc.myFramebuffer->GetDesc().myWidth;
		uint32_t height = m_Desc.myFramebuffer->GetDesc().myHeight;

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = m_Desc.myFramebuffer->GetRenderPass();
		renderPassBeginInfo.framebuffer = m_Desc.myFramebuffer->GetRawBuffer();
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;

		const std::vector<VkClearValue>& clearValues = m_Desc.myFramebuffer->GetClearValues();
		renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassBeginInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(cmd->GetBuffer(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = (float)height;
		viewport.height = -(float)height;
		viewport.width = (float)width;
		viewport.minDepth = (float)0.0f;
		viewport.maxDepth = (float)1.0f;

		vkCmdSetViewport(cmd->GetBuffer(), 0, 1, &viewport);

		VkRect2D scissor = {};
		scissor.extent.width = width;
		scissor.extent.height = height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;

		vkCmdSetScissor(cmd->GetBuffer(), 0, 1, &scissor);
	}

	void Gfx_Pipeline::CmdEndRenderPass(Gfx_CmdBuffer* cmd)
	{
		vkCmdEndRenderPass(cmd->GetBuffer());
	}

	void Gfx_Pipeline::CmdPushConstant(Gfx_CmdBuffer* cmd, ShaderStage stage, uint32_t size, const void* data)
	{
		vkCmdPushConstants(cmd->GetBuffer(), m_Layout, Gfx_VulkanHelpers::GetShaderStage(stage), 0, size, data);
	}

	void Gfx_Pipeline::CmdDrawIndexed(Gfx_CmdBuffer* cmdBuffer, Gfx_VertexBuffer* vb, Gfx_IndexBuffer* ib)
	{
		VkDeviceSize offsets[1] = { 0 };
		VkBuffer vk_vb = vb->GetBuffer().GetRawBuffer();
		VkBuffer vk_ib = ib->GetBuffer().GetRawBuffer();
		VkCommandBuffer cmd = cmdBuffer->GetBuffer();

		vkCmdBindVertexBuffers(cmd, 0, 1, &vk_vb, offsets);
		vkCmdBindIndexBuffer(cmd, vk_ib, 0, VK_INDEX_TYPE_UINT32); // TODO:: add uint16_t
		vkCmdDrawIndexed(cmd, ib->GetCount(), 1, 0, 0, 1);
	}

	void Gfx_Pipeline::CmdDraw(Gfx_CmdBuffer* cmdBuffer, Gfx_VertexBuffer* vb, uint32_t vertextCount)
	{
		VkDeviceSize offsets[1] = { 0 };
		VkBuffer vk_vb = vb->GetBuffer().GetRawBuffer();
		VkCommandBuffer cmd = cmdBuffer->GetBuffer();

		vkCmdBindVertexBuffers(cmd, 0, 1, &vk_vb, offsets);
		vkCmdDraw(cmd, vertextCount, 1, 0, 0);
	}

	void Gfx_Pipeline::CmdDrawMeshIndexed(Gfx_CmdBuffer* cmdBuffer, Gfx_Mesh* mesh, uint32_t instances /*= 1*/)
	{
		VkDeviceSize offsets[1] = { 0 };
		VkBuffer vk_vb = mesh->GetVertexBuffer()->GetBuffer().GetRawBuffer();
		VkBuffer vk_ib = mesh->GetIndexBuffer()->GetBuffer().GetRawBuffer();
		VkCommandBuffer cmd = cmdBuffer->GetBuffer();

		vkCmdBindVertexBuffers(cmd, 0, 1, &vk_vb, offsets);
		vkCmdBindIndexBuffer(cmd, vk_ib, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmd, mesh->GetIndexBuffer()->GetCount(), instances, 0, 0, 0);
	}

	void Gfx_Pipeline::CmdDrawMesh(Gfx_CmdBuffer* cmdBuffer, Gfx_Mesh* mesh, uint32_t instances /*= 1*/)
	{
		VkDeviceSize offsets[1] = { 0 };
		VkBuffer vk_vb = mesh->GetVertexBuffer()->GetBuffer().GetRawBuffer();
		VkBuffer vk_ib = mesh->GetIndexBuffer()->GetBuffer().GetRawBuffer();
		VkCommandBuffer cmd = cmdBuffer->GetBuffer();

		vkCmdBindVertexBuffers(cmd, 0, 1, &vk_vb, offsets);
		vkCmdBindIndexBuffer(cmd, vk_ib, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDraw(cmd, mesh->GetVertexBuffer()->GetVertexCount(), instances, 0, 0);
	}

	void Gfx_Pipeline::CmdBindPipeline(Gfx_CmdBuffer* cmdBuffer)
	{
		VkCommandBuffer cmd = cmdBuffer->GetBuffer();
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipelines[m_DrawMode]);
	}

	void Gfx_Pipeline::CmdBindDescriptor(Gfx_CmdBuffer* cmd, Gfx_Descriptor* another /*= nullptr*/)
	{
		VkDescriptorSet set = another == nullptr ? m_Desc.myDescriptor->GetSet() : another->GetSet();
		vkCmdBindDescriptorSets(cmd->GetBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Layout, 0, 1, &set, 0, nullptr);
	}

	void Gfx_Pipeline::Create(PipelineCreateDesc* desc)
	{
		if (!locIsPipelineCreateDescValid(desc))
		{
			// TODO: assert here
		}

		m_Desc = *desc;
		VkDevice device = Gfx_Context::GetDevice().GetLogicalDevice();

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

		for (DrawMode mode : desc->myDrawModes)
		{
			VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
			inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssemblyState.topology = Gfx_VulkanHelpers::GetTopology(mode);
			inputAssemblyState.primitiveRestartEnable = desc->myPrimitiveRestartEnable;

			VkPipelineRasterizationStateCreateInfo rasterizationState = {};
			rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizationState.polygonMode = Gfx_VulkanHelpers::GetPolygonMode(desc->myPolygonMode);
			rasterizationState.cullMode = Gfx_VulkanHelpers::GetCullMode(desc->myCullMode);
			rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			rasterizationState.depthClampEnable = Gfx_Context::GetDevice().GetDeviceFeatures()->depthClamp;
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
					blendAttachmentState[i].blendEnable = locIsBlendEnableEnabled(desc) ? VK_TRUE : VK_FALSE;
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

			VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineCreateInfo, nullptr, &m_Pipelines[mode]));
		}
	}

	void Gfx_Pipeline::Reload()
	{
		
	}

	void Gfx_Pipeline::Free()
	{
		VK_DESTROY_DEVICE_HANDLE(m_Layout, vkDestroyPipelineLayout);

		for (auto& [key, pipeline] : m_Pipelines)
			VK_DESTROY_DEVICE_HANDLE(pipeline, vkDestroyPipeline);

		m_Pipelines.clear();
	}

	bool Gfx_Pipeline::IsGood() const
	{
		return m_Pipelines.size() > 0;
	}

	void Gfx_Pipeline::SetDrawMode(DrawMode mode)
	{
		m_DrawMode = mode;
	}
}