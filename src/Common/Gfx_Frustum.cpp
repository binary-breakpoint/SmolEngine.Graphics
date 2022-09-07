#include "Gfx_Precompiled.h"
#include "Common/Gfx_Frustum.h"

namespace SmolEngine
{
	void Gfx_Frustum::SetRadius(float value)
	{
		m_Radius = value;
	}

	void Gfx_Frustum::Update(const glm::mat4& matrix)
	{
		m_Planes[side::LEFT].x = matrix[0].w + matrix[0].x;
		m_Planes[side::LEFT].y = matrix[1].w + matrix[1].x;
		m_Planes[side::LEFT].z = matrix[2].w + matrix[2].x;
		m_Planes[side::LEFT].w = matrix[3].w + matrix[3].x;

		m_Planes[RIGHT].x = matrix[0].w - matrix[0].x;
		m_Planes[RIGHT].y = matrix[1].w - matrix[1].x;
		m_Planes[RIGHT].z = matrix[2].w - matrix[2].x;
		m_Planes[RIGHT].w = matrix[3].w - matrix[3].x;

		m_Planes[TOP].x = matrix[0].w - matrix[0].y;
		m_Planes[TOP].y = matrix[1].w - matrix[1].y;
		m_Planes[TOP].z = matrix[2].w - matrix[2].y;
		m_Planes[TOP].w = matrix[3].w - matrix[3].y;

		m_Planes[BOTTOM].x = matrix[0].w + matrix[0].y;
		m_Planes[BOTTOM].y = matrix[1].w + matrix[1].y;
		m_Planes[BOTTOM].z = matrix[2].w + matrix[2].y;
		m_Planes[BOTTOM].w = matrix[3].w + matrix[3].y;

		m_Planes[BACK].x = matrix[0].w + matrix[0].z;
		m_Planes[BACK].y = matrix[1].w + matrix[1].z;
		m_Planes[BACK].z = matrix[2].w + matrix[2].z;
		m_Planes[BACK].w = matrix[3].w + matrix[3].z;

		m_Planes[FRONT].x = matrix[0].w - matrix[0].z;
		m_Planes[FRONT].y = matrix[1].w - matrix[1].z;
		m_Planes[FRONT].z = matrix[2].w - matrix[2].z;
		m_Planes[FRONT].w = matrix[3].w - matrix[3].z;

		for (auto i = 0; i < m_Planes.size(); i++)
		{
			float length = sqrtf(m_Planes[i].x * m_Planes[i].x + m_Planes[i].y * m_Planes[i].y + m_Planes[i].z * m_Planes[i].z);
			m_Planes[i] /= length;
		}
	}

	bool Gfx_Frustum::CheckSphere(const glm::vec3& pos) const
	{
		for (auto i = 0; i < m_Planes.size(); i++)
		{
			if ((m_Planes[i].x * pos.x) + (m_Planes[i].y * pos.y) + (m_Planes[i].z * pos.z) + m_Planes[i].w <= -m_Radius)
				return false;
		}

		return true;
	}

	const std::array<glm::vec4, 6>& Gfx_Frustum::GetPlanes() const
	{
		return m_Planes;
	}
}