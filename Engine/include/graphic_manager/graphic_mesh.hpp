#pragma once
#include <array>
#include <vector>
#include <utility>
#include <algorithm>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "graphic_resource.hpp"

template<typename Coord, typename Index, typename... Features>
struct Mesh
{
	std::vector<Coord> coords;
	std::vector<Index> index;
	std::tuple<std::vector<Features>...> features;
	GLMeshType meshType;
};

template<typename T>
struct GLMeshComponentTrait {};

template<>
struct GLMeshComponentTrait<glm::vec2> {
	static constexpr int dimensionX = 2;
	static constexpr int dimensionY = 1;
	static constexpr GLElementTypes type = GLElementTypes::Float;
	static constexpr int type_size = sizeof(glm::vec2::value_type);
};

template<>
struct GLMeshComponentTrait<glm::vec3> {
	static constexpr int dimensionX = 3;
	static constexpr int dimensionY = 1;
	static constexpr GLElementTypes type = GLElementTypes::Float;
	static constexpr int type_size = sizeof(glm::vec3::value_type);
};

template <>
struct GLMeshComponentTrait<unsigned int> {
	static constexpr int dimensionX = 1;
	static constexpr int dimensionY = 1;
	static constexpr GLElementTypes type = GLElementTypes::UInt;
	static constexpr int type_size = sizeof(unsigned int);
};

template <>
struct GLMeshComponentTrait<glm::mat3> {
	static constexpr int dimensionX = 3;
	static constexpr int dimensionY = 3;
	static constexpr GLElementTypes type = GLElementTypes::Float;
	static constexpr int type_size = sizeof(glm::mat3::value_type);
};

struct IGraphicMesh
{
	virtual void Bind() = 0;
	virtual unsigned long long CountElement() const noexcept = 0;
	virtual GLElementTypes GetElementType() const noexcept = 0;
	virtual GLMeshType GetMeshType() const noexcept = 0;
	virtual void Unbind() = 0;
	virtual ~IGraphicMesh() {};
};

struct IGraphicMeshInstanced : public IGraphicMesh
{
	virtual unsigned long long InstanceCount() const noexcept = 0;
	virtual ~IGraphicMeshInstanced() {};
};

template<typename Coord, typename Index, typename... Features>
class GLGraphicMesh final : public IGraphicMesh
{
private:
	GLVArray VAO;
	GLBuffer EBO;
	GLBuffer VBOCoord;
	GLBuffers<sizeof...(Features)> VBOFeatures;
	unsigned long long m_element_count;
	GLMeshType m_meshType;
public:
	GLGraphicMesh(const Mesh<Coord, Index, Features...>& mesh) : m_element_count(mesh.index.size()),
		m_meshType(mesh.meshType)
	{
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBOCoord);

		int attribPointer = 0;

		glBufferData(GL_ARRAY_BUFFER, sizeof(Coord) * mesh.coords.size(), mesh.coords.data(), GL_STATIC_DRAW);
		for (int i = 0, offset = 0; i < GLMeshComponentTrait<Coord>::dimensionY; ++i)
		{
			int stride = GLMeshComponentTrait<Coord>::dimensionX * GLMeshComponentTrait<Coord>::dimensionY * GLMeshComponentTrait<Coord>::type_size;
			glVertexAttribPointer(attribPointer, GLMeshComponentTrait<Coord>::dimensionX, static_cast<int>(GLMeshComponentTrait<Coord>::type), GL_FALSE, stride, (void*)offset);
			glEnableVertexAttribArray(attribPointer);
			attribPointer++;
			offset += GLMeshComponentTrait<Coord>::dimensionX * GLMeshComponentTrait<Coord>::type_size;
		}

		std::apply([&](auto&... features) {
			int i = 1;
			auto bindAttributes = [&](auto& feature) {
				using FeatureType = typename std::decay_t<decltype(feature)>::value_type;
				glBindBuffer(GL_ARRAY_BUFFER, VBOFeatures[i - 1]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(FeatureType) * feature.size(), feature.data(), GL_STATIC_DRAW);

				for (int j = 0, offset = 0; j < GLMeshComponentTrait<FeatureType>::dimensionY; ++j)
				{
					int stride = GLMeshComponentTrait<FeatureType>::dimensionX * GLMeshComponentTrait<FeatureType>::dimensionY * GLMeshComponentTrait<FeatureType>::type_size;
					glVertexAttribPointer(attribPointer, GLMeshComponentTrait<FeatureType>::dimensionX, static_cast<int>(GLMeshComponentTrait<FeatureType>::type), GL_FALSE, stride, (void*)offset);
					glEnableVertexAttribArray(attribPointer);
					attribPointer++;
					offset += GLMeshComponentTrait<FeatureType>::dimensionX * GLMeshComponentTrait<FeatureType>::type_size;
				}

				++i;
				};

			(bindAttributes(features), ...);
			}, mesh.features);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index) * mesh.index.size(), mesh.index.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	unsigned long long CountElement() const noexcept override
	{
		return m_element_count;
	}

	GLElementTypes GetElementType() const noexcept override
	{
		return GLMeshComponentTrait<Index>::type;
	}
	GLMeshType GetMeshType() const noexcept override
	{
		return m_meshType;
	}
	void Bind() override
	{
		glBindVertexArray(VAO);
	}

	void Unbind() override
	{
		glBindVertexArray(0);
	}
};




template<typename Coord, typename Index, typename InstanceType, typename... Features>
class GLGraphicMeshInstanced final : public IGraphicMeshInstanced
{
private:
	GLVArray VAO;
	GLBuffer EBO;
	GLBuffer VBOCoord;
	GLBuffers<sizeof...(Features)> VBOFeatures;
	GLBuffer VBOInstanceType;
	unsigned long long m_element_count;
	unsigned long long m_instance_count;
	GLMeshType m_meshType;
public:
	GLGraphicMeshInstanced(const Mesh<Coord, Index, Features...>& mesh, std::unique_ptr<BufferAdapter<InstanceType>> instanceInfo) : m_element_count(mesh.index.size()),
		m_meshType(mesh.meshType),
		m_instance_count(instanceInfo->Count())
	{
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBOCoord);

		int attribPointer = 0;

		glBufferData(GL_ARRAY_BUFFER, sizeof(Coord) * mesh.coords.size(), mesh.coords.data(), GL_STATIC_DRAW);
		for (int i = 0, offset = 0 ; i < GLMeshComponentTrait<Coord>::dimensionY; ++i)
		{
			int stride = GLMeshComponentTrait<Coord>::dimensionX * GLMeshComponentTrait<Coord>::dimensionY * GLMeshComponentTrait<Coord>::type_size;
			glVertexAttribPointer(attribPointer, GLMeshComponentTrait<Coord>::dimensionX, static_cast<int>(GLMeshComponentTrait<Coord>::type), GL_FALSE, stride, (void*)offset);
			glEnableVertexAttribArray(attribPointer);
			attribPointer++;
			offset += GLMeshComponentTrait<Coord>::dimensionX * GLMeshComponentTrait<Coord>::type_size;
		}

		std::apply([&](auto&... features) {
			int i = 1;
			auto bindAttributes = [&](auto& feature) {
				using FeatureType = typename std::decay_t<decltype(feature)>::value_type;
				glBindBuffer(GL_ARRAY_BUFFER, VBOFeatures[i - 1]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(FeatureType) * feature.size(), feature.data(), GL_STATIC_DRAW);

				for (int j = 0, offset = 0; j < GLMeshComponentTrait<FeatureType>::dimensionY; ++j)
				{
					int stride = GLMeshComponentTrait<FeatureType>::dimensionX * GLMeshComponentTrait<FeatureType>::dimensionY * GLMeshComponentTrait<FeatureType>::type_size;
					glVertexAttribPointer(attribPointer, GLMeshComponentTrait<FeatureType>::dimensionX, static_cast<int>(GLMeshComponentTrait<FeatureType>::type), GL_FALSE, stride, (void*)offset);
					glEnableVertexAttribArray(attribPointer);
					attribPointer++;
					offset += GLMeshComponentTrait<FeatureType>::dimensionX * GLMeshComponentTrait<FeatureType>::type_size;
				}

				++i;
				};

			(bindAttributes(features), ...);
			}, mesh.features);

		

		constexpr int attribInstancePointer = 1 + sizeof...(Features);

		glBindBuffer(GL_ARRAY_BUFFER, VBOInstanceType);

		instanceInfo->CopyBuffer();

		for (int i = 0, offset = 0; i < GLMeshComponentTrait<InstanceType>::dimensionY; ++i)
		{
			int stride = GLMeshComponentTrait<InstanceType>::dimensionX * GLMeshComponentTrait<InstanceType>::dimensionY * GLMeshComponentTrait<InstanceType>::type_size;
			glVertexAttribPointer(attribPointer, GLMeshComponentTrait<InstanceType>::dimensionX, static_cast<int>(GLMeshComponentTrait<InstanceType>::type), GL_FALSE, stride, (void*)offset);
			glEnableVertexAttribArray(attribPointer);
			glVertexAttribDivisor(attribPointer, 1);
			attribPointer++;
			offset += GLMeshComponentTrait<InstanceType>::dimensionX * GLMeshComponentTrait<InstanceType>::type_size;
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index) * mesh.index.size(), mesh.index.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	unsigned long long CountElement() const noexcept override
	{
		return m_element_count;
	}
	unsigned long long InstanceCount() const noexcept override
	{
		return m_instance_count;
	}

	void UpdateInstanceData(std::unique_ptr<IBufferAdapter> instanceInfo)
	{
		m_instance_count = instanceInfo->Count();
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBOInstanceType);
		instanceInfo->CopyBuffer();
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	GLElementTypes GetElementType() const noexcept override
	{
		return GLMeshComponentTrait<Index>::type;
	}
	GLMeshType GetMeshType() const noexcept override
	{
		return m_meshType;
	}
	void Bind() override
	{
		glBindVertexArray(VAO);
	}

	void Unbind() override
	{
		glBindVertexArray(0);
	}
};