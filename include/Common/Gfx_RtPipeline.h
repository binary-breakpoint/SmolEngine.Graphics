#pragma once
#include "Common/Gfx_Asset.h"

namespace SmolEngine
{
	class Gfx_Shader;
	class Gfx_Texture;
	class Gfx_CmdBuffer;
	class Gfx_Descriptor;
	class Gfx_AccelStructure;

	struct RtPipelineCreateDesc
	{
		Gfx_Shader* myShader = nullptr;
		Gfx_Descriptor* myDescriptor = nullptr;
		uint32_t myMaxRayRecursionDepth = 1;
	};

	class Gfx_RtPipeline final: public Gfx_Asset
	{
	public:
		struct DispatchDesc
		{
			std::optional<ShaderStage> myAnyHitOrCallable;
			ShaderStage myRayMiss = ShaderStage::RayMiss_0;
			ShaderStage myRayHit = ShaderStage::RayCloseHit_0;

			uint32_t myWidth = 0;
			uint32_t myHeight = 0;
		};

		Gfx_RtPipeline();
		~Gfx_RtPipeline();

		void CmdDispatch(Gfx_CmdBuffer* cmd, DispatchDesc* desc);
		void CmdPushConstant(Gfx_CmdBuffer* cmd, ShaderStage stage, uint32_t size, const void* data);
		void CmdBindDescriptor(Gfx_CmdBuffer* cmd, Gfx_Descriptor* another = nullptr);
		void CmdBindPipeline(Gfx_CmdBuffer* cmd);

		void Create(RtPipelineCreateDesc* desc);
		void Free() override;
		bool IsGood() const override;
		Gfx_Shader* GetShader() const { return m_Desc.myShader; }
		Gfx_Descriptor* GetDescriptor() const { return m_Desc.myDescriptor; }

	private:
		RtPipelineCreateDesc m_Desc;
		VkPipelineLayout m_Layout;
		VkPipeline m_Pipeline;
	};
}