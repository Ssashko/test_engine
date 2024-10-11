#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

enum class GLElementTypes {
	Float = GL_FLOAT,
	Int = GL_INT,
	UInt = GL_UNSIGNED_INT,
	Byte = GL_UNSIGNED_BYTE
};

enum class ShaderType
{
	Vertex,
	Geometry,
	Fragment
};

enum class GLMeshType
{
	Triangle = GL_TRIANGLES,
	Line = GL_LINES,
	Point = GL_POINTS
};