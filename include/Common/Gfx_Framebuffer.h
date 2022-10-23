#pragma once
#include "Memory.h"
#include "Common/Gfx_PixelStorage.h"
#include "Common/Gfx_Flags.h"

#include <glm/glm.hpp>

namespace SmolEngine
{
	class Gfx_Sampler;
	class Gfx_CmdBuffer;

	struct FramebufferAttachment
	{
		FramebufferAttachment() = default;
		FramebufferAttachment(Format format, const glm::vec4& color, const std::string& name = "");

		glm::vec4 myColor;
		std::string myName;
		uint32_t myMips = 1;
		Format myFormat;
		LoadOp myLoadOp = LoadOp::LOAD_OP_CLEAR;
		StoreOp myStoreOp = StoreOp::STORE_OP_STORE;
		DepthMode myDepthMode = DepthMode::DEPTH_STENCIL;
	};

	struct FramebufferCreateDesc
	{
		Ref<Gfx_Sampler> mySampler = nullptr;
		glm::uvec2 mySize = {0, 0};

		std::vector<FramebufferAttachment> myAttachments;

		bool myIsTargetsSwapchain = false;
		bool myIsUsedByImGui = false;
	};

	class Gfx_Framebuffer
	{
	public:
		struct Attachment
		{
			void* myImGuiID = nullptr;
			Ref<Gfx_PixelStorage> myPixelStorage;
			VkClearAttachment myClearAttachment;
			VkClearValue myClearValue;
			VkDescriptorImageInfo myImageInfo;
		};

		Gfx_Framebuffer();
		~Gfx_Framebuffer();

		void Create(FramebufferCreateDesc* desc);
		void OnResize(const glm::ivec2& size);
		void Free();

		Ref<Gfx_PixelStorage> GetPixelStorage(const std::string& name);
		Ref<Gfx_PixelStorage> GetPixelStorage(uint32_t index);

		const glm::ivec2& GetSize() const;

		void* GetImGuiTextureID(uint32_t index = 0);
		Attachment* GetAttachment(const std::string& name);
		Attachment* GetAttachment(uint32_t index);
		Attachment* GetDepthAttachment();

		VkRenderPass GetRenderPass() const { return m_RenderPass; }
		VkFramebuffer GetRawBuffer() const;
		const std::vector<VkFramebuffer>& GetRawBuffers() const { return m_FrameBuffers; }
		const std::vector<VkClearValue>& GetClearValues() const { return m_ClearValues; }
		const FramebufferCreateDesc& GetDesc() const { return m_Desc; }

	private:
		VkRenderPass m_RenderPass;
		Attachment* m_DepthAttachment;
		std::vector<Attachment> m_Attachments;
		std::vector<VkFramebuffer> m_FrameBuffers;
		std::vector<VkClearValue> m_ClearValues;
		std::unordered_map<std::string, uint32_t> m_AttachmentsMap;
		FramebufferCreateDesc m_Desc;
	};
}