#include "wall_manager.hpp"
#include <glm/gtx/matrix_transform_2d.hpp>

TMesh GetWallMesh(const Wall& wall)
{
	glm::vec2 shift{ wall.pos1.y - wall.pos2.y, -wall.pos1.x + wall.pos2.x };
	shift = (wall.thickness / 2.f) * glm::normalize(shift);

	std::vector<glm::vec2> vertex = { wall.pos1 + shift, wall.pos1 - shift, wall.pos2 - shift, wall.pos2 + shift };
	std::vector<glm::vec3> colour = { wall.colour, wall.colour, wall.colour, wall.colour };
	std::vector<unsigned int> index = { 0, 1, 2, 2, 3, 0 };
	return TMesh{ vertex, index, std::tuple(colour), GLMeshType::Triangle };
}

glm::mat3 SegmentTransformWithThickness(const glm::vec2& A, const glm::vec2& B,
	const glm::vec2& C, const glm::vec2& D, float thicknessAB, float thicknessCD) {

	

	glm::vec2 dirAB = B - A;
	glm::vec2 dirCD = D - C;
	float lengthAB = glm::length(dirAB);
	float lengthCD = glm::length(dirCD);
	float scaleLength = lengthCD / lengthAB;
	float scaleThickness = thicknessCD / thicknessAB;
	float angleAB = atan2(dirAB.y, dirAB.x);
	float angleCD = atan2(dirCD.y, dirCD.x);
	float rotationAngle = angleCD - angleAB;
	glm::mat3 translationToOrigin = glm::translate(glm::mat3(1.0f), -A);
	glm::mat3 rotationMatrix = glm::rotate(glm::mat3(1.0f), rotationAngle);
	glm::mat3 scaleMatrix = glm::scale(glm::mat3(1.0f), glm::vec2(scaleLength, scaleThickness));
	glm::mat3 translationToTarget = glm::translate(glm::mat3(1.0f), C);
	return translationToTarget * rotationMatrix * scaleMatrix * translationToOrigin;
}