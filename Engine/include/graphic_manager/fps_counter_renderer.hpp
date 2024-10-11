#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include <ft2build.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <memory>
#include FT_FREETYPE_H  

#include "graphic_exception.hpp"
#include "graphic_resource.hpp"
#include "graphic_mesh.hpp"
#include "graphic_shader.hpp"
#include <glm/gtx/matrix_transform_2d.hpp>

class FPSCounterRenderer
{
private:
	enum Chars
	{
		Colon = 10,
		Point = 11,
		F = 12,
		P = 13,
		S = 14,
		Empty = 15,
	};
	struct Character {
		GLTextTexture Texture;
		glm::vec2 Size;
		glm::vec2 Bearing;
		unsigned int Advance;
	} m_charachers[16];
	std::unique_ptr<IProgram> program;
	std::unique_ptr<IGraphicMesh> mesh;
public:
	FPSCounterRenderer();

	void Render(float fps_f, float x = -0.95, float y = 0.95, float scale = 0.001f);
};