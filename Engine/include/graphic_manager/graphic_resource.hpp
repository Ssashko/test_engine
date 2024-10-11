#pragma once

#include "graphic_enums.hpp"
#include "graphic_exception.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <array>
#include <optional>

struct IBufferAdapter
{
	virtual void CopyBuffer() const = 0;
	virtual unsigned long long Count() const noexcept = 0;
	virtual unsigned long long TypeSizeOf() const noexcept = 0;
	virtual ~IBufferAdapter() = 0 {};
};

template <typename T>
class BufferAdapter : public IBufferAdapter
{
private:
	std::vector<T> m_data;
public:
	template <typename U>
	BufferAdapter(U&& data) : m_data(std::forward<U>(data))
	{}

	BufferAdapter()
	{}

	unsigned long long Count() const noexcept override
	{
		return m_data.size();
	}
	unsigned long long TypeSizeOf() const noexcept override
	{
		return sizeof(T);
	}

	void CopyBuffer() const override
	{
		glBufferData(GL_ARRAY_BUFFER, sizeof(T) * m_data.size(), m_data.data(), GL_DYNAMIC_DRAW);
	}
};

struct IUniform
{
	virtual void Bind(unsigned int handle, const std::string& name) const = 0;
	virtual ~IUniform() = 0 {};
};

template <typename T>
class GLUniform : public IUniform
{};

template <>
class GLUniform<glm::mat3> final : public IUniform
{
private:
	glm::mat3 m_mat3;
public:
	GLUniform() : m_mat3(1.0f)
	{}
	template <typename U>
	GLUniform(U&& mat3) : m_mat3(std::forward<U>(mat3))
	{}
	void Bind(unsigned int handle, const std::string& name) const override
	{
		glUniformMatrix3fv(glGetUniformLocation(handle, name.c_str()), 1, GL_FALSE, glm::value_ptr(m_mat3));
	}
};

struct IProgram
{
	virtual void Bind() = 0;
	virtual void Unbind() = 0;
	virtual void SetUniform(const IUniform* uniform, const std::string& name) = 0;
	virtual ~IProgram() = 0 {};
};

template <typename T>
class Resource
{
public:
	Resource(T handle) : m_handle(handle)
	{}
	operator T() const
	{
		return m_handle;
	}
	Resource(const Resource&) = delete;
	Resource& operator=(const Resource&) = delete;

protected:
	Resource() {};
	T m_handle;
};

class GLBuffer final : public Resource<unsigned int>
{
public:
	GLBuffer()
	{
		glGenBuffers(1, &m_handle);
	}
	~GLBuffer()
	{
		glDeleteBuffers(1, &m_handle);
	}
};
template <int size>
class GLBuffers final : public Resource<std::array<unsigned int, size>>
{
public:
	GLBuffers()
	{
		glGenBuffers(size, this->m_handle.data());
	}
	unsigned int operator[](unsigned long long i) const
	{
		return this->m_handle[i];
	}
	~GLBuffers()
	{
		glDeleteBuffers(size, this->m_handle.data());
	}

};
class GLVArray final : public Resource<unsigned int>
{
public:
	GLVArray()
	{
		glGenVertexArrays(1, &m_handle);
	}
	~GLVArray()
	{
		glDeleteVertexArrays(1, &m_handle);
	}

};


class GLShader final : public Resource<unsigned int>
{
public:
	GLShader(ShaderType type, const std::string& shader_source);

	~GLShader()
	{
		glDeleteShader(m_handle);
	}
};

class GLProgram final : public Resource<unsigned int>, public IProgram
{
public:
	GLProgram(unsigned int handle) : Resource(handle)
	{}

	GLProgram(GLProgram&& program) : Resource(std::exchange(program.m_handle, 0)) 
	{}

	GLProgram& operator=(GLProgram&& program)
	{
		m_handle = std::exchange(program.m_handle, 0);
	}
	void SetUniform(const IUniform* uniform, const std::string& name) override
	{
		uniform->Bind(m_handle, name);
	}
	void Bind() override
	{
		glUseProgram(m_handle);
	}
	void Unbind() override
	{
		glUseProgram(0);
	}
	~GLProgram()
	{
		glDeleteProgram(m_handle);
	}
};

class GLProgramBuilder
{
private:
	std::string m_vertex_shader;
	std::string m_geometry_shader;
	std::string m_fragment_shader;
public:
	GLProgramBuilder& AddShader(ShaderType type, std::string shader);
	GLProgram Build();
};


class GLTextTexture : public Resource<unsigned int>
{
public:
	GLTextTexture()
	{
		glGenTextures(1, &m_handle);
	}
	void Load(const unsigned char* data, int width, int height)
	{
		glBindTexture(GL_TEXTURE_2D, m_handle);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

	}

	void BindTextureToUnit(int unit = 0)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, m_handle);
	}
	~GLTextTexture()
	{
		glDeleteTextures(1, &m_handle);
	}
};