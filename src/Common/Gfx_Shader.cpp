#include "Gfx_Precompiled.h"
#include "Common/Gfx_Shader.h"
#include "Common/Gfx_Helpers.h"
#include "Tools/Gfx_ShaderCompiler.h"

namespace Dia
{
	ShaderCreateDesc& Gfx_Shader::GetDesc()
	{
		return m_CreateInfo;
	}

	Gfx_Shader::~Gfx_Shader()
	{
		Free();
	}

	void Gfx_Shader::Create(ShaderCreateDesc* desc)
	{
		m_CreateInfo = *desc;

		const auto loadOrCompile = [this](ShaderCreateDesc* desc, ShaderStage stage, const std::string& path)
		{
			if (path.empty()){ return; }

			auto& binaries = m_Binary[stage];
			std::string cachedPath = Gfx_Helpers::GetCachedPath(path, CachedPathType::Shader);
			if (Gfx_Helpers::IsPathValid(cachedPath))
			{
				Gfx_ShaderCompiler::LoadSPIRV(cachedPath, binaries);
				return;
			}

			ShaderCompileDesc compileDesc{};
			compileDesc.myDefines = desc->myDefines;
			compileDesc.myFilePath = path;
			compileDesc.myStage = stage;

			Gfx_ShaderCompiler::CompileSPIRV(compileDesc, m_Binary[stage]);
		};

		for (auto& [stage, path] : m_CreateInfo.myStages)
			loadOrCompile(desc, stage, path);

		VkDevice device = Gfx_Context::GetDevice().GetLogicalDevice();

		bool raytracingShaders = false;

		// Shader Modules
		for (auto& [stage, data] : m_Binary)
		{
			if (stage == ShaderStage::RayGen)
				raytracingShaders = true;

			VkShaderStageFlagBits vkStage = Gfx_VulkanHelpers::GetShaderStage(stage);
			VkShaderModule shaderModule = nullptr;
			{
				VkShaderModuleCreateInfo shaderModuleCI = {};
				shaderModuleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				shaderModuleCI.codeSize = data.size() * sizeof(uint32_t);
				shaderModuleCI.pCode = data.data();

				VK_CHECK_RESULT(vkCreateShaderModule(device, &shaderModuleCI, nullptr, &shaderModule));
			}

			VkPipelineShaderStageCreateInfo pipelineShaderStageCI{};
			{
				pipelineShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				pipelineShaderStageCI.stage = vkStage;
				pipelineShaderStageCI.pName = desc->myEntryPoint;
				pipelineShaderStageCI.module = shaderModule;

				assert(pipelineShaderStageCI.module != VK_NULL_HANDLE);
			}

			m_ShaderStages.emplace_back(pipelineShaderStageCI);
			m_ShaderModules[stage] = shaderModule;

			m_ShaderIDs[stage] = static_cast<uint32_t>(m_ShaderStages.size());
		}

		// RT shaders
		if (raytracingShaders)
		{
			constexpr auto createGroup = [](const std::map<ShaderStage, uint32_t>& map, ShaderStage type,
				std::vector<VkRayTracingShaderGroupCreateInfoKHR>& out_group)
			{
				const auto& it = map.find(type);
				if (it != map.end())
				{
					uint32_t ID = it->second - 1;

					VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
					shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;

					VkRayTracingShaderGroupTypeKHR flags = type == ShaderStage::RayGen
						|| type == ShaderStage::RayMiss_0
						|| type == ShaderStage::RayMiss_1
						|| type == ShaderStage::RayMiss_2
						|| type == ShaderStage::RayMiss_3
						|| type == ShaderStage::Callable_0
						|| type == ShaderStage::Callable_1
						|| type == ShaderStage::Callable_2
						|| type == ShaderStage::Callable_3 ?
						VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR : VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;

					shaderGroup.type = flags;
					shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
					shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
					shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
					shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;

					switch (type)
					{
					case ShaderStage::RayAnyHit_0:      return;
					case ShaderStage::RayAnyHit_1:      return;
					case ShaderStage::RayAnyHit_2:      return;
					case ShaderStage::RayAnyHit_3:      return;

					case ShaderStage::RayGen:           shaderGroup.generalShader = ID; break;
					case ShaderStage::RayMiss_0:        shaderGroup.generalShader = ID; break;
					case ShaderStage::RayMiss_1:        shaderGroup.generalShader = ID; break;
					case ShaderStage::RayMiss_2:        shaderGroup.generalShader = ID; break;
					case ShaderStage::RayMiss_3:        shaderGroup.generalShader = ID; break;
					case ShaderStage::Callable_0:       shaderGroup.generalShader = ID; break;
					case ShaderStage::Callable_1:       shaderGroup.generalShader = ID; break;
					case ShaderStage::Callable_2:       shaderGroup.generalShader = ID; break;
					case ShaderStage::Callable_3:       shaderGroup.generalShader = ID; break;

					case ShaderStage::RayCloseHit_0:
					{
						shaderGroup.closestHitShader = ID;
						const auto& anyIt = map.find(ShaderStage::RayAnyHit_0);
						if (anyIt != map.end())
							shaderGroup.anyHitShader = anyIt->second - 1;

						break;
					}
					case ShaderStage::RayCloseHit_1:
					{
						shaderGroup.closestHitShader = ID;
						const  auto& anyIt = map.find(ShaderStage::RayAnyHit_1);
						if (anyIt != map.end())
							shaderGroup.anyHitShader = anyIt->second - 1;

						break;
					}
					case ShaderStage::RayCloseHit_2:
					{
						shaderGroup.closestHitShader = ID;
						const auto& anyIt = map.find(ShaderStage::RayAnyHit_2);
						if (anyIt != map.end())
							shaderGroup.anyHitShader = anyIt->second - 1;

						break;
					}
					case ShaderStage::RayCloseHit_3:
					{
						shaderGroup.closestHitShader = ID;
						const auto& anyIt = map.find(ShaderStage::RayAnyHit_3);
						if (anyIt != map.end())
							shaderGroup.anyHitShader = anyIt->second - 1;

						break;
					}
					}

					out_group.push_back(shaderGroup);
				}
			};

			createGroup(m_ShaderIDs, ShaderStage::RayGen, m_ShaderGroupsRT);
			createGroup(m_ShaderIDs, ShaderStage::RayMiss_0, m_ShaderGroupsRT);
			createGroup(m_ShaderIDs, ShaderStage::RayMiss_1, m_ShaderGroupsRT);
			createGroup(m_ShaderIDs, ShaderStage::RayMiss_2, m_ShaderGroupsRT);
			createGroup(m_ShaderIDs, ShaderStage::RayMiss_3, m_ShaderGroupsRT);
			createGroup(m_ShaderIDs, ShaderStage::RayCloseHit_0, m_ShaderGroupsRT);
			createGroup(m_ShaderIDs, ShaderStage::RayCloseHit_1, m_ShaderGroupsRT);
			createGroup(m_ShaderIDs, ShaderStage::RayCloseHit_2, m_ShaderGroupsRT);
			createGroup(m_ShaderIDs, ShaderStage::RayCloseHit_3, m_ShaderGroupsRT);
			createGroup(m_ShaderIDs, ShaderStage::Callable_0, m_ShaderGroupsRT);
			createGroup(m_ShaderIDs, ShaderStage::Callable_1, m_ShaderGroupsRT);
			createGroup(m_ShaderIDs, ShaderStage::Callable_2, m_ShaderGroupsRT);
			createGroup(m_ShaderIDs, ShaderStage::Callable_3, m_ShaderGroupsRT);
		}
	}

	void Gfx_Shader::CreateBindingTable(VkPipeline pipeline)
	{
		constexpr auto addIndexIfExist = [](const std::map<ShaderStage, uint32_t>& map, ShaderStage type, std::vector<uint32_t>& out_index)
		{
			const auto& it = map.find(type);
			if (it != map.end())
			{
				out_index.push_back(it->second - 1);
			}
		};

		constexpr auto createShaderBufferIfExist = [](const std::map<ShaderStage, uint32_t>& map, ShaderStage type, uint32_t stride,
			const std::vector<uint32_t>& indexList, std::unordered_map<ShaderStage, Gfx_Buffer>& buffer_map)
		{
			const VkBufferUsageFlags sbt_buffer_usage_flags = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

			const auto& it = map.find(type);
			if (it != map.end())
			{
				auto& buffer = buffer_map[type];

				BufferCreateDesc bufferDesc{};
				bufferDesc.mySize = stride * indexList.size();
				bufferDesc.myBufferUsage = sbt_buffer_usage_flags;

				buffer.Create(bufferDesc);
			}
		};

		constexpr auto copyHandlesIfExist = [](const std::vector<uint32_t>& indices, std::vector<uint8_t> shader_handle_storage,
			std::unordered_map<ShaderStage, Gfx_Buffer>& buffer_map, ShaderStage type, uint32_t stride, uint32_t handle_size)
		{
			const auto& it = buffer_map.find(type);
			if (it != buffer_map.end())
			{
				auto* pBuffer = static_cast<uint8_t*>(it->second.MapMemory());
				for (uint32_t index = 0; index < static_cast<uint32_t>(indices.size()); index++)
				{
					auto* pStart = pBuffer;
					// Copy the handle
					memcpy(pBuffer, shader_handle_storage.data() + (indices[index] * handle_size), handle_size);
					pBuffer = pStart + stride;        // Jumping to next group
				}

				it->second.UnMapMemory();
			}
		};

		std::vector<uint32_t> rgen_index;
		std::vector<uint32_t> miss_index;
		std::vector<uint32_t> hit_index;
		std::vector<uint32_t> any_hit_index;
		std::vector<uint32_t> callable_index;

		addIndexIfExist(m_ShaderIDs, ShaderStage::RayGen, rgen_index);
		addIndexIfExist(m_ShaderIDs, ShaderStage::RayMiss_0, miss_index);
		addIndexIfExist(m_ShaderIDs, ShaderStage::RayMiss_1, miss_index);
		addIndexIfExist(m_ShaderIDs, ShaderStage::RayMiss_2, miss_index);
		addIndexIfExist(m_ShaderIDs, ShaderStage::RayMiss_3, miss_index);
		addIndexIfExist(m_ShaderIDs, ShaderStage::RayCloseHit_0, hit_index);
		addIndexIfExist(m_ShaderIDs, ShaderStage::RayCloseHit_1, hit_index);
		addIndexIfExist(m_ShaderIDs, ShaderStage::RayCloseHit_2, hit_index);
		addIndexIfExist(m_ShaderIDs, ShaderStage::RayCloseHit_3, hit_index);
		addIndexIfExist(m_ShaderIDs, ShaderStage::RayAnyHit_0, any_hit_index);
		addIndexIfExist(m_ShaderIDs, ShaderStage::RayAnyHit_1, any_hit_index);
		addIndexIfExist(m_ShaderIDs, ShaderStage::RayAnyHit_2, any_hit_index);
		addIndexIfExist(m_ShaderIDs, ShaderStage::RayAnyHit_3, any_hit_index);
		addIndexIfExist(m_ShaderIDs, ShaderStage::Callable_0, callable_index);
		addIndexIfExist(m_ShaderIDs, ShaderStage::Callable_1, callable_index);
		addIndexIfExist(m_ShaderIDs, ShaderStage::Callable_2, callable_index);
		addIndexIfExist(m_ShaderIDs, ShaderStage::Callable_3, callable_index);

		Gfx_VulkanDevice& device = Gfx_Context::GetDevice();

		const uint32_t handle_size = device.rayTracingPipelineProperties.shaderGroupHandleSize;
		const uint32_t handle_alignment = device.rayTracingPipelineProperties.shaderGroupHandleAlignment;
		const uint32_t handle_size_aligned = Gfx_VulkanHelpers::GetAlignedSize(handle_size, handle_alignment);

		createShaderBufferIfExist(m_ShaderIDs, ShaderStage::RayGen, handle_size_aligned, rgen_index, m_BindingTables);
		createShaderBufferIfExist(m_ShaderIDs, ShaderStage::RayMiss_0, handle_size_aligned, miss_index, m_BindingTables);
		createShaderBufferIfExist(m_ShaderIDs, ShaderStage::RayCloseHit_0, handle_size_aligned, hit_index, m_BindingTables);
		createShaderBufferIfExist(m_ShaderIDs, ShaderStage::RayAnyHit_0, handle_size_aligned, any_hit_index, m_BindingTables);
		createShaderBufferIfExist(m_ShaderIDs, ShaderStage::Callable_0, handle_size_aligned, callable_index, m_BindingTables);

		// Copy the pipeline's shader handles into a host buffer
		{
			const auto group_count = static_cast<uint32_t>(rgen_index.size() + miss_index.size() + hit_index.size() + callable_index.size());
			const auto sbt_size = group_count * handle_size_aligned;
			std::vector<uint8_t> shader_handle_storage(sbt_size);

			VK_CHECK_RESULT(device.vkGetRayTracingShaderGroupHandlesKHR(device.GetLogicalDevice(),
				pipeline, 0, group_count, sbt_size, shader_handle_storage.data()));

			copyHandlesIfExist(rgen_index, shader_handle_storage, m_BindingTables, ShaderStage::RayGen, handle_size_aligned, handle_size);
			copyHandlesIfExist(miss_index, shader_handle_storage, m_BindingTables, ShaderStage::RayMiss_0, handle_size_aligned, handle_size);
			copyHandlesIfExist(hit_index, shader_handle_storage, m_BindingTables, ShaderStage::RayCloseHit_0, handle_size_aligned, handle_size);
			copyHandlesIfExist(any_hit_index, shader_handle_storage, m_BindingTables, ShaderStage::RayAnyHit_0, handle_size_aligned, handle_size);
			copyHandlesIfExist(callable_index, shader_handle_storage, m_BindingTables, ShaderStage::Callable_0, handle_size_aligned, handle_size);
		}
	}

	std::vector<VkPipelineShaderStageCreateInfo>& Gfx_Shader::GetShaderStages()
	{
		return m_ShaderStages;
	}

	void Gfx_Shader::Realod()
	{
		Free();
		Create(&m_CreateInfo);
	}

	void Gfx_Shader::Free()
	{
		m_Binary.clear();

		DestroyModules();
	}

	void Gfx_Shader::DestroyModules()
	{
		for (auto& [key, module] : m_ShaderModules)
			VK_DESTROY_DEVICE_HANDLE(module, vkDestroyShaderModule);

		m_ShaderModules.clear();
	}

	bool Gfx_Shader::IsGood() const
	{
		return m_CreateInfo.myStages.size() > 0;
	}

}