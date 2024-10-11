#include "bullet_manager.hpp"

TMesh GetBulletMesh(const Bullet& bullet, int fidelity)
{
	std::vector<glm::vec2> vertex;
	std::vector<glm::vec3> colour;
	std::vector<unsigned int> index;
	vertex.reserve(fidelity + 2);
	colour.reserve(fidelity + 2);
	index.reserve((fidelity + 1) * 3);
	using namespace std::numbers;
	vertex.push_back(bullet.pos);
	colour.push_back(bullet.colour);
	for (int i = 0; i <= fidelity; ++i)
	{
		double angle = 2 * pi * static_cast<double>(i) / fidelity;
		vertex.emplace_back(
			std::cos(angle) * bullet.radius + bullet.pos.x,
			std::sin(angle) * bullet.radius + bullet.pos.y
		);
		colour.push_back(bullet.colour);
	}
	for (int i = 2; i <= fidelity; ++i)
	{
		index.push_back(0);
		index.push_back(i - 1);
		index.push_back(i);
	}
	index.push_back(0);
	index.push_back(fidelity);
	index.push_back(1);
	return TMesh{ vertex, index, std::tuple(colour), GLMeshType::Triangle };
}