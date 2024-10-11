#pragma once
#include <glm/glm.hpp>

#include <functional>

struct Rect
{
	glm::vec2 start;
	glm::vec2 end;
	float height;

};
struct Circle
{
	glm::vec2 pos;
	float radius;
};
struct AABB {
	glm::vec2 max;
	glm::vec2 min;
};

struct CircleRectCollideInfo
{
	glm::vec2 normal_collision;
	unsigned int circle_id;
	unsigned int rect_id;
};


class ColliderRect;
class ColliderCircle;

struct IColliderAABB
{
	virtual const AABB& GetBoundingBox() = 0;
	virtual void Transform(const glm::mat3& transformation) = 0;
	virtual void Test(const IColliderAABB* collider) const = 0;
	virtual void Test(const ColliderRect* collider) const = 0;
	virtual void Test(const ColliderCircle* collider) const = 0;
	virtual unsigned int GetId() const = 0;
	virtual ~IColliderAABB() = 0 {};
};

class ColliderRect : public IColliderAABB
{
	friend ColliderCircle;
private:
	Rect m_rect;
	AABB m_AABB;
	unsigned int m_id;

	void UpdateAABB()
	{
		glm::vec2 v = m_rect.end - m_rect.start;
		glm::vec2 n = m_rect.height / 2.f * glm::normalize(glm::vec2(-v.y, v.x));

		glm::vec2 quadrangle[4] = { m_rect.end - n, m_rect.end + n, m_rect.start - n, m_rect.start + n };

		m_AABB.max = glm::vec2(
			std::max(std::max(quadrangle[0].x, quadrangle[1].x), std::max(quadrangle[2].x, quadrangle[3].x)),
			std::max(std::max(quadrangle[0].y, quadrangle[1].y), std::max(quadrangle[2].y, quadrangle[3].y))
		);
		m_AABB.min = glm::vec2(
			std::min(std::min(quadrangle[0].x, quadrangle[1].x), std::min(quadrangle[2].x, quadrangle[3].x)),
			std::min(std::min(quadrangle[0].y, quadrangle[1].y), std::min(quadrangle[2].y, quadrangle[3].y))
		);
	}
	
public:
	ColliderRect(const Rect& rect, unsigned int id) : m_rect(rect), m_id(id){ UpdateAABB(); }

	unsigned int GetId() const override
	{
		return m_id;
	}
	void Test(const IColliderAABB* collider) const override
	{
		collider->Test(this);
	}
	void Transform(const glm::mat3& transformation) override
	{
		m_rect.start = transformation * glm::vec3(m_rect.start, 1.f);
		m_rect.end = transformation * glm::vec3(m_rect.end, 1.f);
		UpdateAABB();
	}
	void Test(const ColliderRect* collider) const override
	{}
	void Test(const ColliderCircle* collider) const override;
	const AABB& GetBoundingBox() override
	{
		return m_AABB;
	}
};

class ColliderCircle : public IColliderAABB
{
	friend ColliderRect;
private:
	Circle m_circle;
	AABB m_AABB;
	unsigned int m_id;

	void UpdateAABB()
	{
		m_AABB.max = glm::vec2(m_circle.pos.x + m_circle.radius, m_circle.pos.y + m_circle.radius);
		m_AABB.min = glm::vec2(m_circle.pos.x - m_circle.radius, m_circle.pos.y - m_circle.radius);
	}
	static std::vector<std::function<void(CircleRectCollideInfo)>> s_callbacks_circle_rect_collider;
public:
	ColliderCircle(const Circle& circle, unsigned int id) : m_id(id), m_circle(circle) { UpdateAABB(); }

	unsigned int GetId() const override
	{
		return m_id;
	}

	static void OnCollideAll(std::function<void(CircleRectCollideInfo)>&& callback)
	{
		s_callbacks_circle_rect_collider.push_back(std::move(callback));
	}

	void Test(const IColliderAABB* collider) const override
	{
		collider->Test(this);
	}
	void Transform(const glm::mat3& transformation) override
	{
		m_circle.pos = transformation * glm::vec3(m_circle.pos, 1.f);
		UpdateAABB();
	}
	void Test(const ColliderRect* collider) const override;
	void Test(const ColliderCircle* collider) const override
	{}
	const AABB& GetBoundingBox() override
	{
		return m_AABB;
	}
};