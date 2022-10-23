#pragma once

#include "Backend/Gfx_VulkanCore.h"

namespace SmolEngine
{
	struct CmdBufferCreateDesc
	{
		enum class Type
		{
			Graphics,
			Compute
		};

		VkCommandPool myPool = nullptr;
		VkFence myFence = nullptr;
		Type myType = Type::Graphics;
	};

	class Gfx_CmdBuffer
	{
		friend class Gfx_VulkanHelpers;
		friend class Gfx_App;

		enum class State
		{
			Record,
			NeedExecute,
			Wait
		};

	public:
		Gfx_CmdBuffer();
		~Gfx_CmdBuffer();

		void Free();
		bool IsGood() const;
		void Create(CmdBufferCreateDesc* desc);
		void Reset();
		void CmdBeginRecord();
		void CmdEndRecord();

		VkCommandBuffer GetBuffer();
		VkCommandPool GetPool();

	private:
		VkCommandBuffer m_Buffer;
		VkCommandPool m_Pool;
		State m_State;
		bool m_ExternalPool;
	};
}