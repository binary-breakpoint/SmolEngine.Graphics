#pragma once

#include "Common/Gfx_Memory.h"
#include "Common/Gfx_Asset.h"

#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace SmolEngine
{
	struct AnimClipDesc
	{
		float mySpeed = 1.0f;
		bool myLoop = true;
		bool myPlay = false;
	};

	struct AnimClipCreateDesc
	{
		AnimClipDesc myClipDesc;
		std::string mySkeletonPath = "";
		std::string myAnimationPath = "";
		std::string myModelPath = "";
		std::string myName = "";
	};

	struct AnimationClipStorage;

	class Gfx_AnimClip final: public Gfx_Asset
	{
		friend class Gfx_AnimController;
	public:
		Gfx_AnimClip();
		~Gfx_AnimClip();

		void Reset();
		void SetTimeRatio(float ratio);
		bool IsGood() const override;

		float GetDuration() const;
		float GetTimeRatio() const;
		AnimClipCreateDesc& GetDesc();

	private:
		bool Create(const AnimClipCreateDesc& createInfo);
		void CopyJoints(std::vector<glm::mat4>& dist, uint32_t& out_index);
		bool Update();

		Ref<AnimationClipStorage> m_Storage;
		AnimClipCreateDesc m_Info;
		float m_TimeRatio;
		float m_PreviousTimeRatio;
		float m_Duration;
	};
}