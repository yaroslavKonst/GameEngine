#ifndef _PLANET_H
#define _PLANET_H

#include <list>
#include <set>

#include "../VideoEngine/video.h"
#include "../PhysicalEngine/PhysicalEngine.h"

class Segment : public Model, public PhysicalObject
{
public:
	Math::Vec<3> Normal;
	Loader::VertexData Geometry;
	bool Loaded;
	bool Activated;
};

class Planet
{
public:
	Planet(
		double radius,
		Math::Vec<3> position,
		Video* video,
		PhysicalEngine* physicalEngine);
	~Planet();

	void Update(const Math::Vec<3>& playerCoord);

private:
	std::list<Segment*> _segments;
	std::map<Segment*, std::set<Segment*>> _segmentNeighbours;

	std::set<Segment*> _activeSegments;
	std::set<Segment*> _subActiveSegments;

	Math::Mat<4> _matrix;
	Math::Vec<3> _position;

	Video* _video;
	PhysicalEngine* _physicalEngine;

	uint32_t _blockTexture;

	void ActivateSegment(Segment* segment);
	void DeactivateSegment(Segment* segment);

	void LoadSegment(Segment* segment);
	void UnloadSegment(Segment* segment);
};

#endif
