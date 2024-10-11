#include <iostream>
#include <memory>
#include <numbers>
#include <ranges>
#include <random>

#include "engine.hpp"
#include "graphic_manager/graphic_shader.hpp"

#include "bullet_manager.hpp"
#include "wall_manager.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp>


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

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> distr_float(-950.f, 950.f);
	std::uniform_real_distribution<float> distr_float_offset(10.f, 100.f);

	std::ranges::for_each(std::views::iota(0, 10000), [&](auto) {
		glm::vec2 vec = glm::vec2(distr_float(gen), distr_float(gen));
		wallManager->AddWall(vec, glm::vec2(std::min(vec.x + distr_float_offset(gen), 950.f), std::min(vec.y + distr_float_offset(gen), 950.f)), 5);
	});

	std::ranges::for_each(std::views::iota(0, 1000), [&](auto) {
		bulletManager->Fire(glm::vec2{ 0.f, 0.f }, glm::normalize(glm::vec2(distr_float(gen), distr_float(gen))), 800.f, engine.GetCurrentTimeStamp(), 60);
	});

	while (engine.Update());

	return 0;
}