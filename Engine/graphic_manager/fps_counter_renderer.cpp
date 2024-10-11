#include "graphic_manager/fps_counter_renderer.hpp"

FPSCounterRenderer::FPSCounterRenderer()
{
	program = std::make_unique<GLProgram>(
		GLProgramBuilder()
		.AddShader(ShaderType::Vertex, shaders_source::vertex_shader_text)
		.AddShader(ShaderType::Fragment, shaders_source::fragment_shader_text)
		.Build()
	);
	mesh = std::make_unique<GLGraphicMesh<glm::vec2, unsigned int, glm::vec2>>(Mesh<glm::vec2, unsigned int, glm::vec2>{
		{
			{0.f, 1.f},
			{ 0.f, 0.f },
			{ 1.f, 0.f },
			{ 1.f, 1.f }
		},
				{
					0, 1, 2, 0, 2, 3
				},
			std::make_tuple(std::vector<glm::vec2>{
				{0.f, 0.f},
				{ 0.f, 1.f },
				{ 1.f, 1.f },
				{ 1.f, 0.f }
		}),
			GLMeshType::Triangle
	});

	FT_Library ft;
	if (FT_Init_FreeType(&ft))
	{
		throw FTInitException();
	}
	FT_Face face;
	if (FT_New_Face(ft, FONT_NAME, 0, &face))
	{
		throw FTInitException();
	}
	FT_Set_Pixel_Sizes(face, 0, 48);
	const std::string chars = "0123456789:.FPS ";
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	for (char i = 0; i < 16; ++i)
	{
		if (FT_Load_Char(face, chars[i], FT_LOAD_RENDER))
		{
			throw FTInitException();
		}
		m_charachers[i].Texture.Load(
			face->glyph->bitmap.buffer,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows
		);
		m_charachers[i].Size = glm::vec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
		m_charachers[i].Bearing = glm::vec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
		m_charachers[i].Advance = face->glyph->advance.x;
	}
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

void FPSCounterRenderer::Render(float fps_f, float x, float y, float scale)
{
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(1) << fps_f;
	std::string fps = oss.str();

	std::vector<int> chars = { F, P, S, Colon, Empty };
	for (int i = 0; i < fps.size(); ++i)
		if (fps[i] == '.')
			chars.push_back(Point);
		else
			chars.push_back(fps[i] - '0');

	for (int i = 0; i < chars.size(); ++i)
	{
		Character& ch = m_charachers[chars[i]];

		std::unique_ptr<IUniform> transform = std::make_unique<GLUniform<glm::mat3>>(
			glm::translate(glm::mat3(1.f), glm::vec2(x, y))
			* glm::scale(glm::mat3(1.f), glm::vec2(scale))
			* glm::translate(glm::mat3(1.f), glm::vec2(ch.Bearing.x, ch.Bearing.y - ch.Size.y))
			* glm::scale(glm::mat3(1.f), ch.Size)
		);


		program->Bind();
		program->SetUniform(transform.get(), "transformation");
		mesh->Bind();
		ch.Texture.BindTextureToUnit();
		glDrawElements(GL_TRIANGLES,
			mesh->CountElement(),
			GL_UNSIGNED_INT,
			static_cast<void*>(0));
		mesh->Unbind();
		program->Unbind();

		x += (ch.Advance >> 6) * scale;
	}

}