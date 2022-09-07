#pragma once
#include "Memory.h"
#include "Common/Gfx_Asset.h"
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
		Gfx_Sampler* mySampler = nullptr;
		int32_t myWidth = 0;
		int32_t myHeight = 0;
		bool myIsTargetsSwapchain = false;
		bool myIsUsedByImGui = false;
		bool myIsResizable = true;
		bool myIsDepthSampler = false;
		bool myIsAutoSync = true;
		std::vector<FramebufferAttachment> myAttachments;
	};

	class Gfx_Framebuffer final: public Gfx_Asset
	{
	public:
		struct Attachment
		{
			void* myImGuiID = nullptr;
			Gfx_PixelStorage myPixelStorage;
			VkClearAttachment myClearAttachment;
			VkClearValue myClearValue;
			VkDescriptorImageInfo myImageInfo;
		};

		Gfx_Framebuffer();
		~Gfx_Framebuffer();

		void Create(FramebufferCreateDesc* desc);
		void OnResize(uint32_t width, uint32_t height);
		void Free() override;

		void CmdClear(Gfx_CmdBuffer* cmd, const glm::vec4& color, uint32_t index = 0);

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