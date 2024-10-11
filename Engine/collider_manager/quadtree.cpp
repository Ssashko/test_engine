#include "collider_manager/quadtree.hpp"

Quadtree::QuadtreeNode::Quads Quadtree::QuadtreeNode::GetQuad(const AABB& bb) const
{
	glm::vec2 center = m_pos + glm::vec2(m_width / 2);
	if (bb.max.x < center.x)
	{
		if (bb.max.y < center.y)
			return BottomLeft;
		else if (bb.min.y > center.y)
			return TopLeft;
		else
			return None;
	}
	else if (bb.min.x > center.x)
	{
		if (bb.max.y < center.y)
			return BottomRight;
		else if (bb.min.y > center.y)
			return TopRight;
		else
			return None;
	}
	else
		return None;
};

void Quadtree::QuadtreeNode::Subdivide()
{
	assert(IsTerminate());

	quads[TopRight] = std::make_unique<QuadtreeNode>(m_pos + glm::vec2(m_width / 2), m_width / 2);
	quads[TopLeft] = std::make_unique<QuadtreeNode>(m_pos + glm::vec2(0, m_width / 2), m_width / 2);
	quads[BottomRight] = std::make_unique<QuadtreeNode>(m_pos + glm::vec2(m_width / 2, 0), m_width / 2);
	quads[BottomLeft] = std::make_unique<QuadtreeNode>(m_pos, m_width / 2);

	std::unordered_map<unsigned int, AABB> items;
	for (auto& bb : m_items)
	{
		Quads q = GetQuad(bb.second);
		if (q != Quads::None)
			quads[q]->m_items.insert(bb);
		else
			items.insert(bb);
	}
	m_items = std::move(items);
}
void Quadtree::QuadtreeNode::Merge()
{
	assert(!IsTerminate());

	int element_size = 0;
	for (int i = 0; i < 4; ++i)
	{
		if (!quads[i]->IsTerminate())
			return;
		element_size += quads[i]->m_items.size();
	}
	element_size += m_items.size();
	if (element_size <= s_max_el_count)
	{
		for (int i = 0; i < 4; ++i)
		{
			m_items.insert(std::make_move_iterator(quads[i]->m_items.begin()), 
				std::make_move_iterator(quads[i]->m_items.end()));
			quads[i].reset();
		}
	}
}

void Quadtree::QuadtreeNode::Insert(unsigned int id, const AABB& bb, int depth)
{
	if (!Contains(bb))
	{
		volatile int o = 0;
	}
	assert(Contains(bb));

	if (IsTerminate())
	{
		if (m_items.size() < s_max_el_count || depth > s_max_depth)
			m_items.emplace(id, bb);
		else
		{
			Subdivide();
			Insert(id, bb, depth);
		}
	}
	else
	{
		Quads q = GetQuad(bb);
		if (q != Quads::None)
			quads[q]->Insert(id, bb, depth + 1);
		else
			m_items.emplace(id, bb);
	}
}

void Quadtree::QuadtreeNode::Remove(unsigned int id, const AABB& bb, QuadtreeNode* parent)
{
	assert(Contains(bb));
	if (IsTerminate())
	{
		m_items.erase(id);
		if (parent)
			parent->Merge();
	}
	else
	{
		Quads q = GetQuad(bb);
		if (q != Quads::None)
			quads[q]->Remove(id, bb, this);
		else
			m_items.erase(id);
	}
}

void Quadtree::QuadtreeNode::IntersectQuery(const AABB& bb, std::vector<unsigned int>& intersection_ids) const
{

	for (const auto& val_p : m_items)
	{
		if (intersects(val_p.second, bb))
			intersection_ids.push_back(val_p.first);
	}
	if (!IsTerminate())
	{
		for (int i = 0; i < 4; ++i)
			if (quads[i]->Intersects(bb))
				quads[i]->IntersectQuery(bb, intersection_ids);
	}
}