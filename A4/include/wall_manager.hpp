#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include <algorithm>

#include "graphic_manager/graphic_manager.hpp"
#include "collider_manager/collider_handlers.hpp"
#include "collider_manager/collider_manager.hpp"

#include "engine.hpp"

#include "ranges"

using TMesh = Mesh<glm::vec2, unsigned int, glm::vec3>;

struct Wall
{
	glm::vec2 pos1;
	glm::vec2 pos2;
	float thickness;
	glm::vec3 colour;
};

TMesh GetWallMesh(const Wall& wall);
glm::mat3 SegmentTransformWithThickness(const glm::vec2& A, const glm::vec2& B,
	const glm::vec2& C, const glm::vec2& D, float thicknessAB = 0.1f, float thicknessCD = 0.1f);
struct WallData
{
	unsigned int collider_id;
	glm::mat3 transform;
};

template <typename... Extensions>
class WallManager final : public IManager
{
private:
	std::shared_ptr<IGraphicManager> m_graphic_manager;
	std::shared_ptr<ColliderBBManager> m_collider_manager;
	std::unordered_map<unsigned int, WallData> m_wallsData;
	unsigned int m_graphic_id;
	std::vector<WallData> m_exposedWallsData;
	unsigned int m_counter = 0;
public:
	WallManager(const std::tuple<std::shared_ptr<Extensions>...>& extensions, float) :
		m_graphic_manager(std::get<std::shared_ptr<IGraphicManager>>(extensions)),
		m_collider_manager(std::get<std::shared_ptr<ColliderBBManager>>(extensions))
	{
		m_graphic_id = m_graphic_manager->AddEntityInstanced(std::make_unique<GraphicEntityInstanced<glm::vec2, unsigned int, glm::mat3, glm::vec3>>(
			GetWallMesh({
				{0.f, 0.f},
				{1.f, 0.f},
				0.01f,
				glm::vec3{0.5f, 0.5f, 0.f}
				}), "default_instanced_2d"));

		ColliderCircle::OnCollideAll([this](CircleRectCollideInfo info) {
			auto it = m_wallsData.find(info.rect_id);
			if (it != m_wallsData.end())
			{
				m_exposedWallsData.push_back(it->second);
				m_wallsData.erase(it);
			}
		});
	};

	void AddWall(glm::vec2 start, glm::vec2 end, float thickness = 0.01f)
	{
		++m_counter;
		
		unsigned int cid = m_collider_manager->AddEntity(std::make_unique<ColliderRect>(
			ColliderRect(Rect{ start,end,thickness }, m_counter)));

		m_wallsData.emplace(m_counter, WallData{ cid,  ::SegmentTransformWithThickness({0.f, 0.f}, {1.f, 0.f}, start, end, 0.01f, thickness)});
	}

	bool Update(float) override
	{
		
		std::for_each(m_exposedWallsData.begin(), m_exposedWallsData.end(), [&](auto& wallData) {
			m_collider_manager->DeleteEntity(wallData.collider_id);
			});

		if (!m_exposedWallsData.empty())
		{
			std::vector<glm::mat3> instanceData;
			instanceData.reserve(m_wallsData.size());

			std::ranges::copy(
				m_wallsData | std::ranges::views::values
				| std::views::transform([](const WallData& wd) { return wd.transform; }),
				std::back_inserter(instanceData));

			m_graphic_manager->ChangeEntityInstanceTransformation(m_graphic_id, std::make_unique<BufferAdapter<glm::mat3>>(std::move(instanceData)));

			m_exposedWallsData.clear();
		}
		
		return true;
	}
};