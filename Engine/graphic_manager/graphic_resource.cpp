#include "graphic_manager/graphic_resource.hpp"

void CheckShaderErrors(uint32_t shaderHandler) {
	int32_t success;
	std::vector<char> infoLog(4096);
	glGetShaderiv(shaderHandler, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		int32_t writed = 0;
		glGetShaderInfoLog(shaderHandler, 4095, &writed, infoLog.data());
		infoLog[writed] = '/0';

		throw GLShaderCompilationException(infoLog.data());
	}
}

void CheckProgramLinkError(uint32_t shaderHandlerProgram) {
	int32_t success;
	std::vector<char> infoLog(4096);
	glGetProgramiv(shaderHandlerProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		int32_t writed = 0;
		glGetProgramInfoLog(shaderHandlerProgram, 4095, &writed, infoLog.data());
		infoLog[writed] = '/0';

		throw GLProgramLinkingException(infoLog.data());
	}
}

GLShader::GLShader(ShaderType type, const std::string& shader_source)
{
	unsigned int shader_type = 0;
	switch (type)
	{
	case ShaderType::Vertex: shader_type = GL_VERTEX_SHADER; break;
	case ShaderType::Geometry: shader_type = GL_GEOMETRY_SHADER; break;
	case ShaderType::Fragment: shader_type = GL_FRAGMENT_SHADER;
	}
	m_handle = glCreateShader(shader_type);
	const char* c_str = shader_source.c_str();
	glShaderSource(m_handle, 1, &c_str, NULL);
	glCompileShader(m_handle);
	try
	{
		CheckShaderErrors(m_handle);
	}
	catch (...)
	{
		glDeleteShader(m_handle);
		throw;
	}
}

GLProgramBuilder& GLProgramBuilder::AddShader(ShaderType type, std::string shader)
{
	switch (type)
	{
	case ShaderType::Vertex: m_vertex_shader = shader; break;
	case ShaderType::Geometry: m_geometry_shader = shader; break;
	case ShaderType::Fragment: m_fragment_shader = shader; break;
	}
	return *this;
}

GLProgram GLProgramBuilder::Build()
{
	unsigned int handle = glCreateProgram();
	if (m_vertex_shader.empty() || m_fragment_shader.empty())
		throw GLProgramInvalidBuilderException();

	GLShader vertexShader(ShaderType::Vertex, m_vertex_shader);
	GLShader fragmentShader(ShaderType::Fragment, m_fragment_shader);
	std::optional<GLShader> geometryShader;


	glAttachShader(handle, vertexShader);
	glAttachShader(handle, fragmentShader);
	if (m_geometry_shader != "")
	{
		geometryShader.emplace(ShaderType::Geometry, m_geometry_shader);
		glAttachShader(handle, geometryShader.value());
	}
	glLinkProgram(handle);

	try
	{
		CheckProgramLinkError(handle);
	}
	catch (...)
	{
		glDeleteProgram(handle);
		throw;
	}

	return handle;
}

