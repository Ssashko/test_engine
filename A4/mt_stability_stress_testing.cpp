#include <iostream>
#include <memory>
#include <numbers>
#include <ranges>
#include <random>

#include "engine.hpp"
#include "graphic_manager/graphic_shader.hpp"

#include "bullet_manager.hpp"
#include "wall_manager.hpp"

#include <thread>
#include <chrono>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp>

#include "generators.hpp"


int main()
{
	std::shared_ptr<GLGraphicManager> graphic_manager = std::make_shared<GLGraphicManager>();
	graphic_manager->SetModelTransformation(glm::scale(glm::mat3(1.f), glm::vec2(1e-3f)));
	std::shared_ptr<ColliderBBManager> collider_manager = std::make_shared<ColliderBBManager>(1e+3f);
	std::unique_ptr<IProgram> program = std::make_unique<GLProgram>(
		GLProgramBuilder()
		.AddShader(ShaderType::Vertex, shaders_source::vertex_shader_instanced_default_2d)
		.AddShader(ShaderType::Fragment, shaders_source::fragment_shader_default_2d)
		.Build()
	);

	graphic_manager->AddProgram(std::move(program), "default_instanced_2d");

	Engine<ColliderBBManager, IGraphicManager> engine(std::make_tuple(collider_manager, graphic_manager));
	auto bulletManager = engine.AddManager<BulletManager<ColliderBBManager, IGraphicManager>>();
	auto wallManager = engine.AddManager<WallManager<ColliderBBManager, IGraphicManager>>();

	std::ranges::for_each(generateMaze(), [&](auto& pair) {
		wallManager->AddWall(pair.first, pair.second, 8);
		});

	std::thread thread([&]() {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> distr_float(-1, 1);
		while (engine.IsActive())
		{
			std::ranges::for_each(std::views::iota(0, 20), [&](auto) {
				bulletManager->Fire(glm::vec2{ 0.f, 0.f }, glm::normalize(glm::vec2(distr_float(gen), distr_float(gen))), 800.f, engine.GetCurrentTimeStamp(), 8);
				});
			std::this_thread::sleep_for(std::chrono::seconds(4));
		}
		});

	while (engine.Update());

	thread.join();

	return 0;
}