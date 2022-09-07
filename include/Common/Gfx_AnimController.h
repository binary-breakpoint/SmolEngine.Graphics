#pragma once

#include "Common/Gfx_AnimClip.h"

#include <unordered_map>

namespace SmolEngine
{
	class Gfx_AnimController
	{
	public:
		using list = std::unordered_map<std::string, Ref<Gfx_AnimClip>>;

		void Update();
		bool AddClip(const AnimClipCreateDesc& info, const std::string& name, bool active = false);
		void CopyJoints(std::vector<glm::mat4>& ist, uint32_t& out_index);
		bool RemoveClip(const std::string& name);
		bool SetActiveClip(const std::string& name);
		Ref<Gfx_AnimClip> GetClip(const std::string& name);
		Ref<Gfx_AnimClip> GetActiveClip();
		const list& GetClips() const;

	private:
		Ref<Gfx_AnimClip> m_ActiveClip = nullptr;
		std::unordered_map<std::string, Ref<Gfx_AnimClip>> m_Clips;
	};
}