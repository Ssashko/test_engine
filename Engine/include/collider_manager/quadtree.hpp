#pragma once
#include <unordered_map>
#include <memory>

#include "collider_handlers.hpp"
#include "glm/glm.hpp"

inline bool intersects(const AABB& bb1, const AABB& bb2)
{
	return (bb1.max.x > bb2.min.x && bb2.max.x > bb1.min.x && bb1.max.y > bb2.min.y && bb2.max.y > bb1.min.y);
}

class Quadtree
{
private:
	class QuadtreeNode
	{
	private:
		std::unique_ptr<QuadtreeNode> quads[4];
		std::unordered_map<unsigned int, AABB> m_items;
		glm::vec2 m_pos;
		float m_width;
	public:

		static constexpr int s_max_el_count = 20;
		static constexpr int s_max_depth = 5;
		enum Quads {
			None = -1,
			TopLeft = 0,
			TopRight = 1,
			BottomRight = 2,
			BottomLeft = 3
		};
		QuadtreeNode(glm::vec2 pos, float width) : m_pos(pos), m_width(width)
		{};

		bool IsTerminate() const
		{
			return quads[TopLeft] == nullptr;
		}

		bool Intersects(const AABB& bb) const
		{
			return (bb.max.x > m_pos.x && m_pos.x + m_width > bb.min.x 
				&& bb.max.y > m_pos.y && m_pos.y + m_width > bb.min.y);
		}

		bool Contains(const AABB& bb) const
		{
			return m_pos.x < bb.min.x &&  bb.max.x < m_pos.x + m_width &&
				m_pos.y < bb.min.y && bb.max.y < m_pos.y + m_width;
		}
		Quads GetQuad(const AABB& bb) const;
		void Subdivide();
		void Merge();
		void Insert(unsigned int id, const AABB& bb, int depth = 0);

		void Remove(unsigned int id, const AABB& bb, QuadtreeNode* parent = nullptr);
		void IntersectQuery(const AABB& bb, std::vector<unsigned int>& intersection_ids) const;
	} *root;
public:
	Quadtree(glm::vec2 pos = glm::vec2(-1.f), float width = 2) 
		: root(new QuadtreeNode(pos, width))
	{}
	
	void Insert(unsigned id, const AABB& bb)
	{
		if(root->Contains(bb))
			root->Insert(id, bb);
	}
	void Delete(unsigned id, const AABB& bb)
	{
		if (root->Contains(bb))
			root->Remove(id, bb);
	}
	void Update(unsigned id, const AABB& old_bb, const AABB& new_bb)
	{
		Delete(id, old_bb);
		Insert(id, new_bb);
	}
	std::vector<unsigned int> GetIntersection(const AABB& bb) const
	{
		std::vector<unsigned int> intersection;
		root->IntersectQuery(bb, intersection);
		return intersection;
	}
};