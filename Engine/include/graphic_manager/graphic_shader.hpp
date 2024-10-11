#pragma once

#define glsl(s) _glsl(s)
#define _glsl(s) #s

namespace shaders_source {
	static const char* vertex_shader_default_2d = glsl(

		\#version 440 core\n
		layout(location = 0) in vec2 vertex;
	layout(location = 1) in vec3 colour;

	out vec3 v_colour;

	uniform mat3 transformation;
	uniform mat3 model_transformation;

	void main() {
		vec3 pos = model_transformation * transformation * vec3(vertex, 1.0f);
		gl_Position = vec4(pos.x, pos.y, 0.f, 1.0f);
		v_colour = colour;
	}

		);

	static const char* vertex_shader_instanced_default_2d = glsl(

		\#version 440 core\n
		layout(location = 0) in vec2 vertex;
	layout(location = 1) in vec3 colour;
	layout(location = 2) in mat3 transformation;
	out vec3 v_colour;

	uniform mat3 model_transformation;

	void main() {
		vec3 pos = model_transformation * transformation * vec3(vertex, 1.0f);
		gl_Position = vec4(pos.x, pos.y, 0.f, 1.0f);
		v_colour = colour;
	}

		);

	static const char* fragment_shader_default_2d = glsl(

		\#version 440 core\n

		out vec4 FragColor;

	in vec3 v_colour;

	void main() {

		FragColor = vec4(v_colour, 1.0f);
	}

		);


	static const char* vertex_shader_text = glsl(

		\#version 440 core\n
		layout(location = 0) in vec2 vertex;
	layout(location = 1) in vec2 tex_coord;

	out vec2 texCoord;
	uniform mat3 transformation;

	void main() {
		vec3 pos = transformation * vec3(vertex, 1.0f);
		gl_Position = vec4(pos.x, pos.y, 0.f, 1.0f);
		texCoord = tex_coord;
	}
		);

	static const char* fragment_shader_text = glsl(

		\#version 440 core\n
		out vec4 FragColor;

	in vec2 texCoord;
	uniform sampler2D text;
	void main() {

		FragColor = vec4(0.8, 0, 0, texture(text, texCoord).r);
	}

		);

}