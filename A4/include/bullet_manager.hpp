#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include <tuple>
#include <memory>
#include <cmath>
#include <numbers>
#include <tuple>
#include <algorithm>

#include <glm/gtx/matrix_transform_2d.hpp>

#include "graphic_manager/graphic_manager.hpp"
#include "collider_manager/collider_manager.hpp"

#include "engine.hpp"

#include <ranges>

#include <boost/lockfree/queue.hpp>


using TMesh = Mesh<glm::vec2, unsigned int, glm::vec3>;

struct Bullet
{
	glm::vec2 pos;
	float radius;
	glm::vec3 colour;

};

TMesh GetBulletMesh(const Bullet& bullet, int fidelity = 10);

struct BulletData
{
	unsigned int collider_id;
	glm::vec2 speed;
	glm::mat3 transform;
	float time;
	float life_time;
};
template <typename... Extensions>
class BulletManager : public IManager
{
private:
	std::shared_ptr<IGraphicManager> m_graphic_manager;
	std::shared_ptr<ColliderBBManager> m_collider_manager;
	std::unordered_map<unsigned int, BulletData> m_bulletsData;
	unsigned int m_counter = 0;
	float last_time_stamp;
	unsigned int m_graphic_id;

	boost::lockfree::queue<BulletData> bullet_queue{ 1024 };

	void TranslateEntity(BulletData& bullet, const glm::vec2& v_translate)
	{
		auto col_transform = glm::translate(glm::mat3(1.f), v_translate);
		bullet.transform = bullet.transform * col_transform;
		m_collider_manager->TransformEntity(bullet.collider_id, col_transform);
	}
public:
	BulletManager(const std::tuple<std::shared_ptr<Extensions>...>& extensions, float cur_time) :
		m_graphic_manager(std::get<std::shared_ptr<IGraphicManager>>(extensions)),
		m_collider_manager(std::get<std::shared_ptr<ColliderBBManager>>(extensions)),
		last_time_stamp(cur_time)
	{

		m_graphic_id = m_graphic_manager->AddEntityInstanced(std::make_unique<GraphicEntityInstanced<glm::vec2, unsigned int, glm::mat3, glm::vec3>>(GetBulletMesh({
				{0.f, 0.f},
				5.f,
				glm::vec3{0.8f, 0.f, 0.f}
			}, 20), "default_instanced_2d"));

		ColliderCircle::OnCollideAll([this](CircleRectCollideInfo info) {
			m_bulletsData[info.circle_id].speed = glm::reflect(m_bulletsData[info.circle_id].speed, glm::normalize(info.normal_collision));
		});
	};
	
	bool Update(float time) override
	{
		float dt = std::min(time - last_time_stamp, 0.01f);

		BulletData bullet_data;
		while (bullet_queue.pop(bullet_data)) {
			++m_counter;
			glm::vec2 pos = glm::vec2(bullet_data.transform[2][0], bullet_data.transform[2][1]);
			bullet_data.collider_id = m_collider_manager->AddEntity(std::make_unique<ColliderCircle>(
				ColliderCircle({ pos, 0.01f }, m_counter)
			));
			m_bulletsData.emplace(m_counter, bullet_data);
		}

		std::erase_if(m_bulletsData, [this, time](const auto& bulletPair) {
			const BulletData& bullet = bulletPair.second;
			if (time - bullet.time > bullet.life_time) {
				m_collider_manager->DeleteEntity(bullet.collider_id);
				return true;
			}
			return false;
		});

		std::for_each(m_bulletsData.begin(), m_bulletsData.end(), 
			[dt,time, this](auto& bullet_p) { 
				TranslateEntity(bullet_p.second, dt * bullet_p.second.speed);
			}
		);

		last_time_stamp = time;

		std::vector<glm::mat3> instanceData;
		instanceData.reserve(m_bulletsData.size());

		std::ranges::copy(
			m_bulletsData | std::ranges::views::values
			| std::views::transform([](const BulletData& wd) { return wd.transform; }),
			std::back_inserter(instanceData));

		m_graphic_manager->ChangeEntityInstanceTransformation(m_graphic_id, std::make_unique<BufferAdapter<glm::mat3>>(std::move(instanceData)));

		return true;
	};

	void Fire(glm::vec2 pos, glm::vec2 dir, float speed, float time, float life_time)
	{
		while (!bullet_queue.push({ 0, speed * dir, glm::translate(glm::mat3(1.f), pos), time, life_time, }));
	}


};