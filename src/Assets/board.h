#ifndef _BOARD_H
#define _BOARD_H

#include "../VideoEngine/model.h"

class Board : public Model
{
public:
	Board()
	{
		std::vector<glm::vec3> vertices = {
			{0+5, 0, 0},
			{1+5, 0, 0},
			{0+5, 0, 1},
			{1+5, 0, 1},
			{0+5, 0, 0},
			{1+5, 0, 0},
			{0+5, 0, 1},
			{1+5, 0, 1}
		};

		std::vector<glm::vec3> normals = {
			{0, -1, 0},
			{0, -1, 0},
			{0, -1, 0},
			{0, -1, 0},
			{0, 1, 0},
			{0, 1, 0},
			{0, 1, 0},
			{0, 1, 0}
		};

		std::vector<glm::vec2> texCoords = {
			{0, 0},
			{1, 0},
			{0, 1},
			{1, 1},
			{0, 0},
			{1, 0},
			{0, 1},
			{1, 1}
		};

		std::vector<uint32_t> indices = {
			0, 1, 2, 2, 1, 3,
			4, 6, 5, 6, 7, 5
		};


		SetModelVertices(vertices);
		SetModelNormals(normals);
		SetModelTexCoords(texCoords);
		SetModelIndices(indices);

		SetModelMatrix(glm::mat4(1.0f));
		SetModelInnerMatrix(glm::mat4(1.0f));

		glm::mat4 instance1 = glm::mat4(1.0f);
		glm::mat4 instance2 = glm::translate(
			glm::mat4(1.0f),
			glm::vec3(0, 0.2, 0));

		SetModelInstances({instance1, instance2});

		SetDrawEnabled(true);
	}
};

#endif
