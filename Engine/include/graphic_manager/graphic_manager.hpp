#pragma once
#include <string>
#include <algorithm>
#include <optional>
#include <unordered_map>
#include <memory>

#include "graphic_resource.hpp"
#include "graphic_mesh.hpp"
#include "fps_counter_renderer.hpp"

struct IGraphicEntity
{
	virtual std::string GetProgram() const noexcept = 0;
	virtual IUniform* GetTransform() = 0;
	virtual std::string GetMesh() const noexcept = 0;
	virtual void SetTransform(std::unique_ptr<IUniform> transformation) = 0;
	virtual ~IGraphicEntity() = 0 {};
};

struct IGraphicEntityInstanced
{
	virtual std::string GetProgram() const noexcept = 0;
	virtual IGraphicMeshInstanced* GetMesh() = 0;
	virtual unsigned long long InstanceCount() const noexcept = 0;
	virtual void SetTransformInstances(std::unique_ptr<IBufferAdapter> transformations) = 0;
	virtual ~IGraphicEntityInstanced() = 0 {};
};

struct IGraphicManager
{
	virtual void AddProgram(std::unique_ptr<IProgram> graphic_program, const std::string& shader_name) = 0;
	virtual void AddMesh(std::unique_ptr<IGraphicMesh> graphic_mesh, const std::string& mesh_name) = 0;
	virtual unsigned int AddEntity(std::unique_ptr<IGraphicEntity> graphic_entity) = 0;
	virtual unsigned int AddEntityInstanced(std::unique_ptr<IGraphicEntityInstanced> graphic_entity) = 0;
	virtual void ChangeEntityTransformation(unsigned int graphic_entity_id, std::unique_ptr<IUniform> transformation) = 0;
	virtual void ChangeEntityInstanceTransformation(unsigned int graphic_entity_id,std::unique_ptr<IBufferAdapter> transformations) = 0;
	virtual void DeleteEntity(unsigned int graphic_entity_id) = 0;
	virtual bool Update(float time) = 0;
	virtual ~IGraphicManager() {};
};

class GLGraphicManager final : public IGraphicManager
{
private:
	GLFWwindow* window;

	std::unordered_map<int, std::unique_ptr<IGraphicEntity>> m_graphic_entities;
	std::unordered_map<int, std::unique_ptr<IGraphicEntityInstanced>> m_graphic_entities_instanced;
	unsigned int m_counter = 0;

	std::unordered_map<std::string, std::unique_ptr<IProgram>> m_graphic_programs;
	std::unordered_map<std::string, std::unique_ptr<IGraphicMesh>> m_graphic_meshes;
	std::unique_ptr<IUniform> m_model_transformation;

	std::unique_ptr<FPSCounterRenderer> m_fps_renderer;
	int m_frameCount = 0;
	float m_last_time_stamp = 0;
	float m_fps = 0;
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
	};
	
	void RenderFPS(float time);
public:
	GLGraphicManager(int init_width = 800, int init_height = 800);

	template <typename T>
	void SetModelTransformation(const T& model)
	{
		m_model_transformation = std::make_unique<GLUniform<T>>(model);
	}

	void AddProgram(std::unique_ptr<IProgram> graphic_program,const std::string& shader_name) override
	{
		m_graphic_programs.emplace(shader_name, std::move(graphic_program));
	}
	void AddMesh(std::unique_ptr<IGraphicMesh> graphic_mesh, const std::string& mesh_name) override
	{
		m_graphic_meshes.emplace(mesh_name, std::move(graphic_mesh));
	}
	unsigned int AddEntity(std::unique_ptr<IGraphicEntity> graphic_entity) override
	{
		++m_counter;
		m_graphic_entities.emplace(m_counter, std::move(graphic_entity));
		return m_counter;
	}
	unsigned int AddEntityInstanced(std::unique_ptr<IGraphicEntityInstanced> graphic_entity) override
	{
		++m_counter;
		m_graphic_entities_instanced.emplace(m_counter, std::move(graphic_entity));
		return m_counter;
	}
	void ChangeEntityTransformation(unsigned int graphic_entity_id, std::unique_ptr<IUniform> transformation) override
	{
		m_graphic_entities[graphic_entity_id]->SetTransform(std::move(transformation));
	}
	void DeleteEntity(unsigned int graphic_entity_id) override
	{
		m_graphic_entities.erase(graphic_entity_id);
	}

	void ChangeEntityInstanceTransformation(unsigned int graphic_entity_id, std::unique_ptr<IBufferAdapter> transformations) override
	{
		m_graphic_entities_instanced[graphic_entity_id]->SetTransformInstances(std::move(transformations));
	}

	bool Update(float time) override;

	~GLGraphicManager()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}
};

class GraphicEntity final : public IGraphicEntity
{
private:
	std::string m_mesh_name;
	std::unique_ptr<IUniform> m_transform;
	std::string m_shader_name;
public:

	GraphicEntity(const std::string& mesh_name, const std::string& shader_name, const glm::mat3& transform = glm::mat3(1.f)) :
		m_mesh_name(mesh_name),
		m_shader_name(shader_name),
		m_transform(std::make_unique<GLUniform<glm::mat3>>(transform))
	{}

	std::string GetProgram() const noexcept override
	{
		return m_shader_name;
	}
	IUniform* GetTransform() override
	{
		return m_transform.get();
	}
	std::string GetMesh() const noexcept override
	{
		return m_mesh_name;
	}
	void SetTransform(std::unique_ptr<IUniform> transformation) override
	{
		m_transform = std::move(transformation);
	}
};

template<typename Coord, typename Index, typename InstanceType, typename... Features>
class GraphicEntityInstanced final : public IGraphicEntityInstanced
{
private:
	GLGraphicMeshInstanced<Coord, Index, InstanceType, Features...> m_gl_mesh;
	std::string m_shader_name;
public:

	GraphicEntityInstanced(const Mesh<Coord, Index, Features...>& mesh, const std::string& shader_name) :
		m_gl_mesh(mesh, std::make_unique<BufferAdapter<InstanceType>>()),
		m_shader_name(shader_name)
	{}
	unsigned long long InstanceCount() const noexcept override
	{
		return m_gl_mesh.InstanceCount();
	}
	std::string GetProgram() const noexcept override
	{
		return m_shader_name;
	}
	IGraphicMeshInstanced* GetMesh() override
	{
		return &m_gl_mesh;
	}
	void SetTransformInstances(std::unique_ptr<IBufferAdapter> transformations) override
	{
		m_gl_mesh.UpdateInstanceData(std::move(transformations));
	}
};