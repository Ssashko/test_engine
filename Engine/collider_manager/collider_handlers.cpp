#include "collider_manager/collider_handlers.hpp"
#include <optional>
#include <algorithm>

std::vector<std::function<void(CircleRectCollideInfo)>> ColliderCircle::s_callbacks_circle_rect_collider;

std::optional<glm::vec2> GetNormalCollision(const Rect& r, const Circle& c)
{
	glm::vec2 rect_axis = r.end - r.start;

	float t = std::clamp(glm::dot(rect_axis, c.pos - r.start) / glm::dot(rect_axis, rect_axis), 0.f, 1.f);
	glm::vec2 nearest_axis_point = r.start + t * rect_axis;
	float square_dist = glm::dot(c.pos - nearest_axis_point, c.pos - nearest_axis_point);
	if (square_dist < (c.radius + r.height) * (c.radius + r.height))
		return c.pos - nearest_axis_point;
	return {};
}


void ColliderCircle::Test(const ColliderRect* collider) const
{
	std::optional<glm::vec2> n = GetNormalCollision(collider->m_rect, m_circle);
	if (n.has_value())
		std::for_each(s_callbacks_circle_rect_collider.begin(), s_callbacks_circle_rect_collider.end(), [n, this, collider](auto& callback) {
		callback({ n.value(), this->GetId(), collider->GetId() });
			});
}

void ColliderRect::Test(const ColliderCircle* collider) const
{
	collider->Test(this);
}