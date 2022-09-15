#include "Gfx_Precompiled.h"
#include "Common/Gfx_AnimClip.h"

#include "Tools/Gfx_AnimImporter.h"
#include "Tools/Gfx_MeshImporter.h"

#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/animation/runtime/skeleton_utils.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/base/io/archive.h>
#include <ozz/base/io/stream.h>

#include <ozz/base/maths/soa_transform.h>
#include <ozz/base/maths/math_ex.h>
#include <ozz/base/containers/vector.h>
#include <ozz-animation/include/ozz/animation/runtime/local_to_model_job.h>

#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <glm/gtc/type_ptr.inl>

namespace SmolEngine
{
	struct AnimationClipStorage
	{
		ozz::animation::Skeleton m_Skeleton;
		ozz::animation::Animation m_Clip;
		ozz::animation::SamplingCache m_Cache;
		ozz::vector<ozz::math::SoaTransform> m_Locals;
		ozz::vector<ozz::math::Float4x4> m_Models;
		std::vector<glm::mat4> m_InverseBindMatrices;
	};

	void LoadSkeleton(const char* _filename, ozz::animation::Skeleton* _skeleton) 
	{
		ozz::io::File file(_filename, "rb");
		GFX_ASSERT_MSG(file.opened(), "Failed to open skeleton file")

		ozz::io::IArchive archive(&file);
		GFX_ASSERT_MSG(archive.TestTag<ozz::animation::Skeleton>(), "Failed to load skeleton instance from file: " + std::string(_filename))

		archive >> *_skeleton;
	}

	void LoadAnimation(const char* _filename, ozz::animation::Animation* _animation)
	{
		ozz::io::File file(_filename, "rb");
		GFX_ASSERT_MSG(file.opened(), "Failed to open skeleton file")

		ozz::io::IArchive archive(&file);
		GFX_ASSERT_MSG(archive.TestTag<ozz::animation::Animation>(), "Failed to load animation instance from file: " + std::string(_filename))

		archive >> *_animation;
	}

	bool Gfx_AnimClip::Create(const AnimClipCreateDesc& createInfo)
	{
		m_Storage = std::make_shared<AnimationClipStorage>();

		LoadAnimation(createInfo.myAnimationPath.c_str(), &m_Storage->m_Clip);
		LoadSkeleton(createInfo.mySkeletonPath.c_str(), &m_Storage->m_Skeleton);

		if (Gfx_MeshImporter::ImportInverseBindMatrices(createInfo.myModelPath, m_Storage->m_InverseBindMatrices))
		{
			const int numSoaJoints = m_Storage->m_Skeleton.num_soa_joints();
			const int numTracks = m_Storage->m_Clip.num_tracks();
			const int numJoints = m_Storage->m_Skeleton.num_joints();

			if (numTracks == numJoints)
			{
				m_Storage->m_Models.resize(numJoints);
				m_Storage->m_Cache.Resize(numJoints);
				m_Storage->m_Locals.resize(numSoaJoints);

				m_Info = createInfo;
				m_Duration = m_Storage->m_Clip.duration();
			}

			return true;
		}
		
		return false;
	}

	void Gfx_AnimClip::CopyJoints(std::vector<glm::mat4>& dist, uint32_t& out_index)
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(m_Storage->m_Models.size()); ++i)
		{
			glm::mat4& mBone = dist[out_index];

			ozz::math::StorePtr(m_Storage->m_Models[i].cols[0], glm::value_ptr(mBone[0]));
			ozz::math::StorePtr(m_Storage->m_Models[i].cols[1], glm::value_ptr(mBone[1]));
			ozz::math::StorePtr(m_Storage->m_Models[i].cols[2], glm::value_ptr(mBone[2]));
			ozz::math::StorePtr(m_Storage->m_Models[i].cols[3], glm::value_ptr(mBone[3]));

			mBone *= m_Storage->m_InverseBindMatrices[i];
			out_index++;
		}
	}

	bool Gfx_AnimClip::IsGood() const
	{
		return m_Storage != nullptr;
	}

	AnimClipCreateDesc& Gfx_AnimClip::GetDesc()
	{
		return m_Info;
	}

	float Gfx_AnimClip::GetDuration() const
	{
		return m_Duration;
	}

	float Gfx_AnimClip::GetTimeRatio() const
	{
		return m_TimeRatio;
	}

	void Gfx_AnimClip::Reset()
	{
		m_PreviousTimeRatio = m_TimeRatio = 0.f;
		m_Info.myClipDesc.mySpeed = 1.f;
		m_Info.myClipDesc.myPlay = true;
		m_Info.myClipDesc.myLoop = true;
	}

	void Gfx_AnimClip::SetTimeRatio(float ratio)
	{
		m_PreviousTimeRatio = m_TimeRatio;

		if (m_Info.myClipDesc.myLoop)
			m_TimeRatio = ratio - floorf(ratio);
		else
			m_TimeRatio = ozz::math::Clamp(0.f, ratio, 1.f);
	}

	bool Gfx_AnimClip::Update()
	{
		const float deltaTime = Gfx_Context::GetSingleton()->GetDeltaTime();
		const auto& storage = m_Storage;

		float new_time = m_TimeRatio;
		if (m_Info.myClipDesc.myPlay) {
			new_time = m_TimeRatio + deltaTime * m_Info.myClipDesc.mySpeed / m_Duration;
		}

		SetTimeRatio(new_time);

		// Samples optimized animation at t = animation_time_.
		ozz::animation::SamplingJob sampling_job;
		sampling_job.animation = &storage->m_Clip;
		sampling_job.cache = &storage->m_Cache;
		sampling_job.ratio = m_TimeRatio;
		sampling_job.output = make_span(storage->m_Locals);
		if (!sampling_job.Run())
		{
			return false;
		}

		// Converts from local space to model space matrices.
		ozz::animation::LocalToModelJob ltm_job;
		ltm_job.skeleton = &storage->m_Skeleton;
		ltm_job.input = make_span(storage->m_Locals);
		ltm_job.output = make_span(storage->m_Models);
		if (!ltm_job.Run())
		{
			return false;
		}

		return true;
	}

	bool AnimClipCreateDesc::Load(const std::string& filePath)
	{
		std::stringstream storage;
		std::ifstream file(filePath);

		GFX_ASSERT_MSG(file, "Could not open the file: " + filePath)

		storage << file.rdbuf();
		{
			cereal::JSONInputArchive input{ storage };
			input(myClipDesc.myLoop, myClipDesc.myPlay, myClipDesc.mySpeed,
				mySkeletonPath, myAnimationPath, myModelPath, myName);
		}

		return true;
	}

	bool AnimClipCreateDesc::Save(const std::string& filePath)
	{
		std::stringstream storage;
		{
			cereal::JSONOutputArchive output{ storage };
			serialize(output);
		}

		std::ofstream myfile(filePath);
		if (myfile.is_open())
		{
			myfile << storage.str();
			myfile.close();
			return true;
		}

		return false;
	}
}