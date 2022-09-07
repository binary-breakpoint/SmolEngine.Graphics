#pragma once
#include "Common/Gfx_Asset.h"

namespace SmolEngine
{
	class Gfx_Shader;
	class Gfx_Descriptor;
	class Gfx_CmdBuffer;

	struct CompPipelineCreateDesc
	{
		Gfx_Shader* myShader = nullptr;
		Gfx_Descriptor* myDescriptor = nullptr;
	};

	class Gfx_CompPipeline final: public Gfx_Asset
	{
	public:
		Gfx_CompPipeline();
		~Gfx_CompPipeline();

		void CmdDispatch(Gfx_CmdBuffer* cmd, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ = 1);
		void CmdPushConstant(Gfx_CmdBuffer* cmd, ShaderStage stage, uint32_t size, const void* data);
		void CmdBindDescriptor(Gfx_CmdBuffer* cmd, Gfx_Descriptor* another = nullptr);
		void CmdBindPipeline(Gfx_CmdBuffer* cmd);

		void Create(CompPipelineCreateDesc* desc);
		void Free() override;
		bool IsGood() const override;
		Gfx_Shader* GetShader() const { return m_Desc.myShader; }
		Gfx_Descriptor* GetDescriptor() const { return m_Desc.myDescriptor; }

	private:
		CompPipelineCreateDesc m_Desc;
		VkPipelineLayout m_Layout;
		VkPipeline m_Pipeline;
	};
}