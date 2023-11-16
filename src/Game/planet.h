#ifndef _PLANET_H
#define _PLANET_H

#include <list>

#include "../VideoEngine/video.h"
#include "../PhysicalEngine/object.h"

class Segment : public Model, public Object
{
public:
	glm::vec3 Normal;

private:
};

class Planet
{
public:
	Planet(float radius, Video* video);
	~Planet();

private:
	std::list<Segment*> _segments;

	glm::mat4 _matrix;

	Video* _video;

	std::list<uint32_t> _blockModel;
	uint32_t _blockTexture;
};

#endif
