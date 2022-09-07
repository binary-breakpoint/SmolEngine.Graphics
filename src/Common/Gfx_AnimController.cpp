#include "Gfx_Precompiled.h"
#include "Common/Gfx_AnimController.h"

namespace SmolEngine
{
	bool Gfx_AnimController::AddClip(const AnimClipCreateDesc& info, const std::string& name, bool active)
	{
		bool found = m_Clips.find(name) != m_Clips.end();
		if (!found)
		{
			Ref<Gfx_AnimClip> clip = std::make_shared<Gfx_AnimClip>();
			if (clip->Create(info))
			{
				m_Clips[name] = clip;
				if (active)
					m_ActiveClip = clip;

				return true;
			}
		}

		return false;
	}

	bool Gfx_AnimController::RemoveClip(const std::string& name)
	{
		const auto& it = m_Clips.find(name);
		if (it != m_Clips.end())
		{
			if (it->second == m_ActiveClip)
				m_ActiveClip = nullptr;

			m_Clips.erase(name);
			return true;
		}

		return false;
	}

	bool Gfx_AnimController::SetActiveClip(const std::string& name)
	{
		const auto& it = m_Clips.find(name);
		if (it != m_Clips.end())
		{
			m_ActiveClip = it->second;
			return true;
		}

		return false;
	}

	Ref<Gfx_AnimClip> Gfx_AnimController::GetClip(const std::string& name)
	{
		const auto& it = m_Clips.find(name);
		if (it != m_Clips.end())
		{
			return  it->second;
		}

		return nullptr;
	}

	Ref<Gfx_AnimClip> Gfx_AnimController::GetActiveClip()
	{
		return m_ActiveClip;
	}

	const std::unordered_map<std::string, Ref<Gfx_AnimClip>>& Gfx_AnimController::GetClips() const
	{
		return m_Clips;
	}

	void Gfx_AnimController::Update()
	{
		m_ActiveClip->Update();
	}

	void Gfx_AnimController::CopyJoints(std::vector<glm::mat4>& dist, uint32_t& out_index)
	{
		m_ActiveClip->CopyJoints(dist, out_index);
	}
}