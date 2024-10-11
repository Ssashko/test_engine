#pragma once

#include <exception>
#include <string>

class GLFWInitException : public std::exception
{

};
class FTInitException : public std::exception
{

};
class GladLoadException : public std::exception
{

};
class GLProgramInvalidBuilderException : public std::exception
{

};

class GLShaderCompilationException : public std::exception
{
	std::string m_message;
public:
	GLShaderCompilationException(const char* message) : m_message(message)
	{};
	const char* what() const override
	{
		return m_message.c_str();
	}
};

class GLProgramLinkingException : public std::exception
{
	std::string m_message;
public:
	GLProgramLinkingException(const char* message) : m_message(message)
	{};
	const char* what() const override
	{
		return m_message.c_str();
	}
};