#pragma once
#include "Common/Gfx_Memory.h"
#include "Common/Gfx_Asset.h"
#include "Common/Gfx_Buffer.h"
#include "Backend/Gfx_VulkanHelpers.h"

#include <unordered_map>
#include <string>
#include <glm/glm.hpp>
#include <map>

namespace Dia
{
	struct ShaderCreateDesc
	{
		const char* myEntryPoint = "main";
		std::map<ShaderStage, std::string> myStages;
		std::map<std::string, bool> myDefines;
	};

	class Gfx_Shader final: public Gfx_Asset
	{
		friend class Gfx_Pipeline;
		friend class Gfx_RtPipeline;
		friend class Gfx_CompPipeline;
	public:
		~Gfx_Shader();

		void Create(ShaderCreateDesc* desc);
		void Free() override;
		void Realod();
		bool IsGood() const override;

		ShaderCreateDesc& GetDesc();
		std::vector<VkPipelineShaderStageCreateInfo>& GetShaderStages();
		void CreateBindingTable(VkPipeline pipeline);
		void DestroyModules();

	private:
		ShaderCreateDesc m_CreateInfo;
		std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> m_ShaderGroupsRT;
		std::unordered_map<ShaderStage, VkShaderModule> m_ShaderModules;
		std::unordered_map<ShaderStage, Gfx_Buffer> m_BindingTables;
		std::map<ShaderStage, std::vector<uint32_t>> m_Binary;
		std::map<ShaderStage, uint32_t> m_ShaderIDs;
	};
}