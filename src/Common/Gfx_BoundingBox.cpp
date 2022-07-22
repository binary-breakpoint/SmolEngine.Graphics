#include "Gfx_Precompiled.h"
#include "Common/Gfx_BoundingBox.h"

#include <glm/detail/type_vec3.hpp>
#include <glm/mat4x4.hpp>

namespace Dia
{
	Gfx_BoundingBox::Gfx_BoundingBox()
	{
		Reset();
	}

	void Gfx_BoundingBox::MinPoint(const glm::vec3& val)
	{
		this->m_Boundaries[0] = min(m_Boundaries[0], val);
		this->m_Center = (m_Boundaries[1] + m_Boundaries[0]) * 0.5f;
		this->m_Extent = (m_Boundaries[1] - m_Boundaries[0]) * 0.5f;
	}

	void Gfx_BoundingBox::MaxPoint(const glm::vec3& val)
	{
		this->m_Boundaries[1] = max(m_Boundaries[1], val);
		this->m_Center = (m_Boundaries[1] + m_Boundaries[0]) * 0.5f;
		this->m_Extent = (m_Boundaries[1] - m_Boundaries[0]) * 0.5f;
	}

	void Gfx_BoundingBox::Transform(const glm::mat4x4& model)
	{
		if (!m_Original) { m_Original = std::make_shared<Gfx_BoundingBox>(*this); }

		auto min = m_Original->m_Boundaries[0];
		auto max = m_Original->m_Boundaries[1];
		glm::vec3 corners[8] =
		{
			min,
			max,
			glm::vec3(min.x, min.y, max.z),
			glm::vec3(min.x, max.y, min.z),
			glm::vec3(max.x, min.y, min.z),
			glm::vec3(min.x, max.y, max.z),
			glm::vec3(max.x, min.y, max.z),
			glm::vec3(max.x, max.y, min.z),
		};

		for (int i = 0; i < 8; i++)
		{
			auto transformed = glm::vec3(model * glm::vec4(corners[i], 1.0f));
			m_Boundaries[0] = glm::min(m_Boundaries[0], transformed);
			m_Boundaries[1] = glm::max(m_Boundaries[1], transformed);
		}

		m_Center = (m_Boundaries[1] + m_Boundaries[0]) * 0.5f;
		m_Extent = (m_Boundaries[1] - m_Boundaries[0]) * 0.5f;
	}

	void Gfx_BoundingBox::Reset()
	{
		m_Boundaries[1] = glm::vec3(std::numeric_limits<float>::lowest());
		m_Boundaries[0] = glm::vec3(std::numeric_limits<float>::infinity());

		if (m_Original)
		{
			m_Original.reset();
		}
	}

	const glm::vec3& Gfx_BoundingBox::MinPoint(bool transformed) const
	{
		if (transformed)
		{
			return m_Boundaries[0];
		}

		return !m_Original ? m_Boundaries[0] : m_Original->m_Boundaries[0];
	}

	const glm::vec3& Gfx_BoundingBox::MaxPoint(bool transformed) const
	{
		if (transformed)
		{
			return m_Boundaries[1];
		}

		return !m_Original ? m_Boundaries[1] : m_Original->m_Boundaries[1];
	}

	const glm::vec3& Gfx_BoundingBox::Center(bool transformed) const
	{
		if (transformed)
		{
			return m_Center;
		}

		return !m_Original ? m_Center : m_Original->m_Center;
	}

	const glm::vec3& Gfx_BoundingBox::Extent(bool transformed) const
	{
		if (transformed)
		{
			return m_Extent;
		}

		return !m_Original ? m_Extent : m_Original->m_Extent;
	}
}