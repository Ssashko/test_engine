#include "graphic_manager/graphic_manager.hpp"

GLGraphicManager::GLGraphicManager(int init_width, int init_height)
{
	if (!glfwInit()) {
		throw GLFWInitException();
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(init_width, init_height, "Engine", NULL, NULL);
	if (!window) {
		glfwTerminate();
		throw GLFWInitException();
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		throw GladLoadException();
	}

	glViewport(0, 0, init_width, init_height);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	m_fps_renderer = std::make_unique<FPSCounterRenderer>();
	
	glEnable(GL_MULTISAMPLE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

bool GLGraphicManager::Update(float time)
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	std::for_each(m_graphic_entities.begin(), m_graphic_entities.end(),
		[this](auto& entity_pair) {
			auto entity = entity_pair.second.get();
			auto program = m_graphic_programs.find(entity->GetProgram())->second.get();
			program->Bind();
			program->SetUniform(entity->GetTransform(), "transformation");
			if(m_model_transformation)
				program->SetUniform(m_model_transformation.get(), "model_transformation");
			auto& mesh = m_graphic_meshes[entity->GetMesh()];
			mesh->Bind();
			glDrawElements(static_cast<int>(mesh->GetMeshType()),
				mesh->CountElement(),
				static_cast<int>(mesh->GetElementType()),
				static_cast<void*>(0));
			mesh->Unbind();
			program->Unbind();
		});

	std::for_each(m_graphic_entities_instanced.begin(), m_graphic_entities_instanced.end(),
		[this](auto& entity_pair) {
			auto entity = entity_pair.second.get();
			auto program = m_graphic_programs.find(entity->GetProgram())->second.get();
			program->Bind();
			if (m_model_transformation)
				program->SetUniform(m_model_transformation.get(), "model_transformation");
			auto mesh = entity->GetMesh();
			mesh->Bind();
			glDrawElementsInstanced(static_cast<int>(mesh->GetMeshType()),
				mesh->CountElement(),
				static_cast<int>(mesh->GetElementType()),
				static_cast<void*>(0),
				mesh->InstanceCount());
			mesh->Unbind();
			program->Unbind();
		});

	RenderFPS(time);
	glfwSwapBuffers(window);
	glfwPollEvents();
	
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwWindowShouldClose(window))
		return false;
	return true;
}
void GLGraphicManager::RenderFPS(float time)
{
	float elapsed = time - m_last_time_stamp;
	m_frameCount++;
	if (elapsed > 1.f)
	{
		m_fps = m_frameCount / elapsed;
		m_last_time_stamp = time;
		m_frameCount = 0;
		
	}
	m_fps_renderer->Render(m_fps);
}