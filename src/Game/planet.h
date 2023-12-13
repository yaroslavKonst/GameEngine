#ifndef _PLANET_H
#define _PLANET_H

#include <list>
#include <set>

#include "../VideoEngine/video.h"
#include "../PhysicalEngine/CollisionEngine.h"

class Segment : public Model, public Object
{
public:
	glm::vec3 Normal;
	Loader::VertexData Geometry;
	bool Loaded;
	bool Activated;
};

class Planet
{
public:
	Planet(
		float radius,
		glm::vec3 position,
		Video* video,
		CollisionEngine* collisionEngine);
	~Planet();

	void Update(glm::vec3 playerCoord);

private:
	std::list<Segment*> _segments;
	std::map<Segment*, std::set<Segment*>> _segmentNeighbours;

	std::set<Segment*> _activeSegments;
	std::set<Segment*> _subActiveSegments;

	glm::mat4 _matrix;
	glm::vec3 _position;

	Video* _video;
	CollisionEngine* _collisionEngine;

	uint32_t _blockTexture;

	void ActivateSegment(Segment* segment);
	void DeactivateSegment(Segment* segment);

	void LoadSegment(Segment* segment);
	void UnloadSegment(Segment* segment);
};

#endif
