#pragma once
#include "collider_handlers.hpp"
#include "quadtree.hpp"

#include <unordered_map>
#include <unordered_set>
#include <memory>

class ColliderBBManager final
{
private:
	std::unordered_map<int, std::unique_ptr<IColliderAABB>> m_colliders;
	Quadtree quadtree;

	std::unordered_set<int> m_changedCollider;
	unsigned int m_counter = 0;
public:
	ColliderBBManager(float scale) :
		quadtree(glm::vec2(-scale), 2.f * scale)
	{};
	unsigned int AddEntity(std::unique_ptr<IColliderAABB> collider);
	void TransformEntity(unsigned int collider_id, const glm::mat3& transformation);
	void DeleteEntity(unsigned int collider_id);
	bool Update(float time);
};