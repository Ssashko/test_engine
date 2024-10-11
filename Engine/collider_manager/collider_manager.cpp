#include "collider_manager/collider_manager.hpp"


unsigned int ColliderBBManager::AddEntity(std::unique_ptr<IColliderAABB> collider)
{
	++m_counter;
	quadtree.Insert(m_counter, collider->GetBoundingBox());
	m_colliders.emplace(m_counter, std::move(collider));
	//m_changedCollider.emplace(m_counter);
	return m_counter;
}

void ColliderBBManager::TransformEntity(unsigned int collider_id, const glm::mat3& transformation)
{
	auto collider_it = m_colliders.find(collider_id);
	AABB old_aabb = collider_it->second->GetBoundingBox();
	collider_it->second->Transform(transformation);
	AABB new_aabb = collider_it->second->GetBoundingBox();
	quadtree.Update(collider_id, old_aabb, new_aabb);
	m_changedCollider.emplace(collider_id);
}

void ColliderBBManager::DeleteEntity(unsigned int collider_id)
{
	auto collider_it = m_colliders.find(collider_id);
	quadtree.Delete(collider_id, collider_it->second->GetBoundingBox());
	m_colliders.erase(collider_it);
}

bool ColliderBBManager::Update(float time)
{
	auto changedCollider = std::move(m_changedCollider);
	for (auto col_indexA : changedCollider)
	{
		auto it = m_colliders.find(col_indexA);
		if (it != m_colliders.end())
		{
			const auto& colliderA = it->second;
			for (auto potential_col = quadtree.GetIntersection(colliderA->GetBoundingBox());
				auto col_indexB : potential_col)
				colliderA->Test(m_colliders[col_indexB].get());
		}
	}

	return true;
}