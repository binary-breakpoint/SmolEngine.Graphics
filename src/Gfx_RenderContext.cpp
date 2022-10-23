#include "Gfx_Precompiled.h"
#include "Gfx_RenderContext.h"

namespace SmolEngine
{
	Gfx_RenderContext* Gfx_RenderContext::s_Instance = nullptr;

	Gfx_RenderContext::Gfx_RenderContext()
	{
		s_Instance = this;

		SamplerCreateDesc samplerDesc{};
		m_DefaultSampler = CreateSampler(samplerDesc);
	}

	Ref<Gfx_Shader> Gfx_RenderContext::CreateShader(ShaderCreateDesc& desc, const std::string& debugName)
	{
		Ref<Gfx_Shader> shader = std::make_shared<Gfx_Shader>();
		shader->Create(&desc);

		if (!debugName.empty())
		{
			const auto& it = s_Instance->m_Shaders.find(debugName);
			if (it == s_Instance->m_Shaders.end())
			{
				s_Instance->m_Shaders[debugName] = shader;
			}
		}

		return shader;
	}

	Ref<Gfx_Framebuffer>  Gfx_RenderContext::CreateFramebuffer(FramebufferCreateDesc& desc, const std::string& debugName)
	{
		Ref<Gfx_Framebuffer> fb = std::make_shared<Gfx_Framebuffer>();
		fb->Create(&desc);

		if (!debugName.empty())
		{
			uint32_t numAttachments = static_cast<uint32_t>(fb->GetDesc().myAttachments.size());
			for (uint32_t i = 0; i < numAttachments; ++i)
			{
				std::string id = debugName + "_" + std::to_string(i);
				const auto& it = s_Instance->m_PixelStorages.find(id);
				if (it == s_Instance->m_PixelStorages.end())
				{
					auto attachment = fb->GetAttachment(i);

					if (attachment && attachment != fb->GetDepthAttachment())
					{
						void* descriptor = attachment->myImGuiID;
						GFX_ASSERT(descriptor)

						s_Instance->m_PixelStorages[id] = attachment->myPixelStorage;
					}
				}
			}
		}

		return fb;
	}

	Ref<Gfx_Descriptor> Gfx_RenderContext::CreateDescriptor(DescriptorCreateDesc& desc, const std::string& debugName)
	{
		Ref<Gfx_Descriptor> descriptor = std::make_shared<Gfx_Descriptor>();
		descriptor->Create(&desc);
		return descriptor;
	}

	Ref<Gfx_Texture> Gfx_RenderContext::CreateTexture(TextureCreateDesc& desc, const std::string& debugName)
	{
		Ref<Gfx_Texture> texture = std::make_shared<Gfx_Texture>();
		texture->Create(&desc);

		if (!debugName.empty())
		{
			const auto& it = s_Instance->m_PixelStorages.find(debugName);
			if (it == s_Instance->m_PixelStorages.end())
			{
				s_Instance->m_PixelStorages[debugName] = texture->GetPixelStorage();
			}
		}

		return texture;
	}

	Ref<Gfx_Sampler> Gfx_RenderContext::CreateSampler(SamplerCreateDesc& desc, const std::string& debugname)
	{
		Ref<Gfx_Sampler> sampler = std::make_shared<Gfx_Sampler>();
		sampler->Create(&desc);
		return sampler;
	}

	Ref<Gfx_PixelStorage> Gfx_RenderContext::CreatePixelStorage(PixelStorageCreateDesc& desc, const std::string& debugName)
	{
		Ref<Gfx_PixelStorage> pixStorage = std::make_shared<Gfx_PixelStorage>();
		pixStorage->Create(&desc);
		return pixStorage;
	}

	Ref<Gfx_Buffer> Gfx_RenderContext::CreateBuffer(BufferCreateDesc& desc, const std::string& debugName)
	{
		Ref<Gfx_Buffer> buffer = std::make_shared<Gfx_Buffer>();
		buffer->Create(desc);

		if (!debugName.empty())
		{
			const auto& it = s_Instance->m_Buffers.find(debugName);
			if (it == s_Instance->m_Buffers.end())
			{
				s_Instance->m_Buffers[debugName] = buffer;
			}
		}

		return buffer;
	}

	Ref<Gfx_Pipeline> Gfx_RenderContext::CreateGraphicsPipeline(GraphicsPipelineCreateDesc& desc, const std::string& debugName)
	{
		Ref<Gfx_GraphicsPipeline> pipeline = std::make_shared<Gfx_GraphicsPipeline>();
		pipeline->Create(&desc);
		return pipeline;
	}

	Ref<Gfx_Pipeline> Gfx_RenderContext::CreateComputePipeline(ComputePipelineCreateDesc& desc, const std::string& debugName)
	{
		Ref<Gfx_ComputePipeline> pipeline = std::make_shared<Gfx_ComputePipeline>();
		pipeline->Create(&desc);
		return pipeline;
	}

	Ref<Gfx_Pipeline> Gfx_RenderContext::CreateRaytarcingPipeline(RaytracingPipelineCreateDesc& desc, const std::string& debugName)
	{
		Ref<Gfx_RaytracingPipeline> rtPipeline = std::make_shared<Gfx_RaytracingPipeline>();
		rtPipeline->Create(&desc);
		return rtPipeline;
	}

	Ref<Gfx_Mesh> Gfx_RenderContext::CreateMesh(const std::string& filePath, const TransformDesc& transform, const std::string& debugNane)
	{
		Ref<Gfx_Mesh> mesh = std::make_shared<Gfx_Mesh>();
		mesh->LoadFromFile(filePath, transform);
		return mesh;
	}

	Ref<Gfx_Sampler> Gfx_RenderContext::GetDefaultSampler()
	{
		return s_Instance->m_DefaultSampler;
	}

	void Gfx_RenderContext::CmdPushConstants(const Ref<Gfx_RenderPass>& renderPass, ShaderStage stage, uint32_t size, const void* data)
	{
		Ref<Gfx_CmdBuffer>& cmd = renderPass->myCmd;
		GFX_ASSERT(cmd)

		vkCmdPushConstants(cmd->GetBuffer(), renderPass->myPipeline->GetLayout(), Gfx_VulkanHelpers::GetShaderStage(stage),
			0, size, data);
	}

	void Gfx_RenderContext::CmdBindDescriptor(const Ref<Gfx_RenderPass>& renderPass, const Ref<Gfx_Descriptor>& another)
	{
		Ref<Gfx_CmdBuffer>& cmd = renderPass->myCmd;
		GFX_ASSERT(cmd)

		VkDescriptorSet set = another == nullptr ? renderPass->myDescriptor->GetSet() : another->GetSet();
		vkCmdBindDescriptorSets(cmd->GetBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->myPipeline->GetLayout(), 
			0, 1, &set, 0, nullptr);
	}

	void Gfx_RenderContext::CmdBindPipeline(const Ref<Gfx_RenderPass>& renderPass)
	{
		Ref<Gfx_CmdBuffer>& cmd = renderPass->myCmd;
		GFX_ASSERT(cmd)

		vkCmdBindPipeline(cmd->GetBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->myPipeline->GetPipeline());
	}

	void Gfx_RenderContext::CmdRayDispatch(const Ref<Gfx_RenderPass>& renderPass, RayDispatchDesc* desc)
	{
		Ref<Gfx_CmdBuffer>& cmd = renderPass->myCmd;
		GFX_ASSERT(cmd)

		std::unordered_map<ShaderStage, Gfx_Buffer>& bindingTable = renderPass->myShader->m_BindingTables;
		const Gfx_VulkanDevice& device = Gfx_App::GetDevice();

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
		if (desc->myAnyHitOrCallable.has_value())
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
			desc->mySize.x,
			desc->mySize.y,
			1);

	}

	void Gfx_RenderContext::CmdDispatch(const Ref<Gfx_RenderPass>& renderPass, uint32_t groupCountX,
		uint32_t groupCountY, uint32_t groupCountZ)
	{
		Ref<Gfx_CmdBuffer>& cmd = renderPass->myCmd;
		GFX_ASSERT(cmd)

		vkCmdDispatch(cmd->GetBuffer(), groupCountX, groupCountY, groupCountZ);
	}

	void Gfx_RenderContext::CmdBeginRenderPass(const Ref<Gfx_RenderPass>& renderPass)
	{
		Ref<Gfx_CmdBuffer>& cmd = renderPass->myCmd;
		GFX_ASSERT(cmd)

		glm::uvec2 viewportSize = renderPass->myRenderTarget->GetSize();

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = renderPass->myRenderTarget->GetRenderPass();
		renderPassBeginInfo.framebuffer = renderPass->myRenderTarget->GetRawBuffer();
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = viewportSize.x;
		renderPassBeginInfo.renderArea.extent.height = viewportSize.y;

		const std::vector<VkClearValue>& clearValues = renderPass->myRenderTarget->GetClearValues();
		renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassBeginInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(cmd->GetBuffer(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = (float)viewportSize.y;
		viewport.height = -(float)viewportSize.y;
		viewport.width = (float)viewportSize.x;
		viewport.minDepth = (float)0.0f;
		viewport.maxDepth = (float)1.0f;

		vkCmdSetViewport(cmd->GetBuffer(), 0, 1, &viewport);

		VkRect2D scissor = {};
		scissor.extent.width = viewportSize.x;
		scissor.extent.height = viewportSize.y;
		scissor.offset.x = 0;
		scissor.offset.y = 0;

		vkCmdSetScissor(cmd->GetBuffer(), 0, 1, &scissor);
	}

	void Gfx_RenderContext::CmdEndRenderPass(const Ref<Gfx_RenderPass>& renderPass)
	{
		Ref<Gfx_CmdBuffer>& cmd = renderPass->myCmd;
		GFX_ASSERT(cmd)

		vkCmdEndRenderPass(cmd->GetBuffer());
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

	void Gfx_Pipeline::CmdDraw(Gfx_CmdBuffer* cmdBuffer, uint32_t vertextCount, Gfx_VertexBuffer* vb)
	{
		VkCommandBuffer cmd = cmdBuffer->GetBuffer();
		if (vb != nullptr)
		{
			VkDeviceSize offsets[1] = { 0 };
			VkBuffer buffer = vb->GetBuffer().GetRawBuffer();
			vkCmdBindVertexBuffers(cmd, 0, 1, &buffer, offsets);
		}

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
}