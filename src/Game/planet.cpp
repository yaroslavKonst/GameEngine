#include "planet.h"

#include <vector>

#include "../Logger/logger.h"

struct Vec3
{
	float x;
	float y;
	float z;

	Vec3()
	{
		x = 0;
		y = 0;
		z = 0;
	}

	Vec3(const glm::vec3& vec)
	{
		x = vec.x;
		y = vec.y;
		z = vec.z;
	}

	bool operator<(const Vec3& v) const
	{
		if (x != v.x) {
			return x < v.x;
		}

		if (y != v.y) {
			return y < v.y;
		}

		if (z != v.z) {
			return z < v.z;
		}

		return false;
	}

	bool operator!=(const Vec3& v) const
	{
		if (x != v.x) {
			return true;
		}

		if (y != v.y) {
			return true;
		}

		if (z != v.z) {
			return true;
		}

		return false;
	}

	bool operator==(const Vec3& v) const
	{
		return !operator!=(v);
	}

	operator glm::vec3() const
	{
		return glm::vec3(x, y, z);
	}
};

struct Triangle
{
	Vec3 P1;
	Vec3 P2;
	Vec3 P3;

	Vec3 Center;
	float Radius;

	bool operator<(const Triangle& t) const
	{
		if (P1 != t.P1) {
			return P1 < t.P1;
		}

		if (P2 != t.P2) {
			return P2 < t.P2;
		}

		if (P3 != t.P3) {
			return P3 < t.P3;
		}

		return false;
	}

	const Vec3& operator[](int32_t idx) const
	{
		while (idx < 0) {
			idx += 3;
		}

		idx = idx % 3;

		if (idx == 0) {
			return P1;
		}

		if (idx == 1) {
			return P2;
		}

		if (idx == 2) {
			return P3;
		}

		throw std::runtime_error("Index out of range.");
	}

	bool NonRedundant() const
	{
		return P1 != P2 && P1 != P3 && P2 != P3;
	}

	void SetParams()
	{
		Center = ((glm::vec3)P1 + (glm::vec3)P2 + (glm::vec3)P3) / 3.0f;
		Radius = std::max(
			std::max(
				glm::length((glm::vec3)Center - (glm::vec3)P1),
				glm::length((glm::vec3)Center - (glm::vec3)P2)),
			glm::length((glm::vec3)Center - (glm::vec3)P3));
	}
};

static glm::vec3 AngleToPoint(float lat, float lon, float rad)
{
	float latRad = glm::radians(lat);
	float lonRad = glm::radians(lon);

	glm::vec3 dir = {
		sinf(lonRad) * cosf(latRad),
		cosf(lonRad) * cosf(latRad),
		sinf(latRad)
	};

	dir *= rad;

	return dir;
}

static bool Neighbours(const Triangle& t1, const Triangle& t2)
{
	float dist = glm::length((glm::vec3)t1.Center - (glm::vec3)t2.Center);

	if (dist > t1.Radius + t2.Radius) {
		return false;
	}

	uint32_t match = 0;

	for (size_t idx1 = 0; idx1 < 3; ++idx1) {
		for (size_t idx2 = 0; idx2 < 3; ++idx2) {
			if (t1[idx1] == t2[idx2]) {
				++match;
			}
		}
	}

	return match > 1;
}

Planet::Planet(float radius, Video* video)
{
	_video = video;

	_matrix = glm::translate(glm::mat4(1.0), glm::vec3(0, 0, 2));

	std::map<float, std::vector<float>> angles;

	Logger::Verbose() << "Planet: Building surface.";

	float latStep = 2;

	for (
		float angleLat = -90.0 + latStep / 2.0;
		angleLat <= 90.0 - latStep / 2.0;
		angleLat += latStep)
	{
		int lonAngleCount = ceilf(360.0 / latStep *
			fabs(cosf(glm::radians(angleLat))));

		float lonStep = 360.0 / lonAngleCount;

		Logger::Verbose() << "Planet: Lat: " << angleLat <<
			", point count: " << lonAngleCount;

		std::vector<float> dirs;

		for (float angleLon = 0; angleLon < 360; angleLon += lonStep) {
			dirs.push_back(angleLon);
		}

		angles[angleLat] = dirs;
	}

	Logger::Verbose() << "Planet: Vertices built.";

	std::vector<Triangle> triangles;
	typedef std::vector<Triangle>::iterator TrianglePtr;

	float lonBase = angles.begin()->second.front();
	float latBase = angles.begin()->first;

	for (
		auto lonIt = angles.begin()->second.begin() + 1;
		lonIt != angles.begin()->second.end() - 1;
		++lonIt)
	{
		glm::vec3 p1 = AngleToPoint(
			latBase,
			lonBase,
			radius);

		glm::vec3 p2 = AngleToPoint(
			latBase,
			*lonIt,
			radius);

		glm::vec3 p3 = AngleToPoint(
			latBase,
			*(lonIt + 1),
			radius);

		Triangle t;
		t.P1 = p1;
		t.P2 = p2;
		t.P3 = p3;

		if (t.NonRedundant()) {
			t.SetParams();
			triangles.push_back(t);
		}
	}

	lonBase = angles.rbegin()->second.front();
	latBase = angles.rbegin()->first;

	for (
		auto lonIt = angles.rbegin()->second.begin() + 1;
		lonIt != angles.rbegin()->second.end() - 1;
		++lonIt)
	{
		glm::vec3 p1 = AngleToPoint(
			latBase,
			lonBase,
			radius);

		glm::vec3 p2 = AngleToPoint(
			latBase,
			*lonIt,
			radius);

		glm::vec3 p3 = AngleToPoint(
			latBase,
			*(lonIt + 1),
			radius);

		Triangle t;
		t.P1 = p2;
		t.P2 = p1;
		t.P3 = p3;

		if (t.NonRedundant()) {
			t.SetParams();
			triangles.push_back(t);
		}
	}

	auto latIt1 = angles.begin();
	auto latIt2 = angles.begin();
	++latIt2;

	for (; latIt2 != angles.end(); ++latIt1, ++latIt2) {
		auto lonIt1 = latIt1->second.begin();
		auto lonIt2 = latIt2->second.begin();

		float lon1 = *lonIt1;
		float lon2 = *lonIt2;

		++lonIt1;
		++lonIt2;

		while (
			lonIt1 != latIt1->second.end() ||
			lonIt2 != latIt2->second.end())
		{
			bool advL1;

			if (lonIt1 != latIt1->second.end() &&
				lonIt2 != latIt2->second.end())
			{
				advL1 = *lonIt1 <= *lonIt2;
			} else if (lonIt1 != latIt1->second.end()) {
				advL1 = true;
			} else {
				advL1 = false;
			}

			if (advL1) {
				float prevL = *lonIt1;
				++lonIt1;

				glm::vec3 p1 = AngleToPoint(
					latIt1->first,
					prevL,
					radius);

				glm::vec3 p2 = AngleToPoint(
					latIt2->first,
					lon2,
					radius);

				glm::vec3 p3 = AngleToPoint(
					latIt1->first,
					lon1,
					radius);

				lon1 = prevL;

				Triangle t;
				t.P1 = p2;
				t.P2 = p1;
				t.P3 = p3;

				if (t.NonRedundant()) {
					t.SetParams();
					triangles.push_back(t);
				}
			} else {
				float prevL = *lonIt2;
				++lonIt2;

				glm::vec3 p1 = AngleToPoint(
					latIt1->first,
					lon1,
					radius);

				glm::vec3 p2 = AngleToPoint(
					latIt2->first,
					prevL,
					radius);

				glm::vec3 p3 = AngleToPoint(
					latIt2->first,
					lon2,
					radius);

				lon2 = prevL;

				Triangle t;
				t.P1 = p2;
				t.P2 = p1;
				t.P3 = p3;

				if (t.NonRedundant()) {
					t.SetParams();
					triangles.push_back(t);
				}
			}
		}

		glm::vec3 p1 = AngleToPoint(
			latIt1->first,
			lon1,
			radius);

		glm::vec3 p2 = AngleToPoint(
			latIt2->first,
			lon2,
			radius);

		glm::vec3 p3 = AngleToPoint(
			latIt1->first,
			latIt1->second.front(),
			radius);

		Triangle t;
		t.P1 = p1;
		t.P2 = p2;
		t.P3 = p3;

		if (t.NonRedundant()) {
			t.SetParams();
			triangles.push_back(t);
		}

		p1 = AngleToPoint(
			latIt2->first,
			lon2,
			radius);

		p2 = AngleToPoint(
			latIt2->first,
			latIt2->second.front(),
			radius);

		p3 = AngleToPoint(
			latIt1->first,
			latIt1->second.front(),
			radius);

		t.P1 = p1;
		t.P2 = p2;
		t.P3 = p3;

		if (t.NonRedundant()) {
			t.SetParams();
			triangles.push_back(t);
		}
	}

	Logger::Verbose() << "Planet: Triangle count: " << triangles.size();

	Logger::Verbose() << "Planet: Neighbour map building.";

	std::map<TrianglePtr, std::set<TrianglePtr>> neighbourTriangles;

	for (auto iter = triangles.begin(); iter != triangles.end(); ++iter) {
		neighbourTriangles[iter] = std::set<TrianglePtr>();
	}

	size_t index = 0;
	for (auto iter = triangles.begin(); iter != triangles.end(); ++iter) {
		Logger::Verbose() << "Planet: Neighbour map building: " <<
			index << " / " << triangles.size();

		for (auto it = iter + 1; it != triangles.end(); ++it) {
			if (Neighbours(*iter, *it)) {
				neighbourTriangles[iter].insert(it);
				neighbourTriangles[it].insert(iter);
			}
		}

		++index;
	}

	Logger::Verbose() << "Planet: Neighbour map built.";

	Logger::Verbose() << "Planet: Clusterization.";

	size_t clusterSizeLimit = 100;

	std::list<std::list<TrianglePtr>> clusters;
	std::set<TrianglePtr> freeTriangles;

	for (auto iter = triangles.begin(); iter != triangles.end(); ++iter) {
		freeTriangles.insert(iter);
	}

	while (!freeTriangles.empty()) {
		std::list<TrianglePtr> cluster;
		cluster.push_back(*freeTriangles.begin());
		freeTriangles.erase(cluster.front());

		std::set<TrianglePtr> clusterNeighbours;

		for (auto neighbour : neighbourTriangles[cluster.front()]) {
			if (
				freeTriangles.find(neighbour) !=
				freeTriangles.end())
			{
				clusterNeighbours.insert(neighbour);
			}
		}

		while (!clusterNeighbours.empty() &&
			cluster.size() < clusterSizeLimit)
		{
			glm::vec3 clusterCenter = {0, 0, 0};

			for (auto vertex : cluster) {
				clusterCenter += (glm::vec3)vertex->Center;
			}

			clusterCenter /= (float)cluster.size();

			float cDist = radius * 2;

			auto cVert = clusterNeighbours.begin();

			auto vert = clusterNeighbours.begin();
			while (vert != clusterNeighbours.end()) {
				float dist = glm::length(
					clusterCenter -
					(glm::vec3)(*vert)->Center);

				if (dist < cDist) {
					cDist = dist;
					cVert = vert;
				}

				++vert;
			}

			cluster.push_back(*cVert);
			freeTriangles.erase(*cVert);

			clusterNeighbours.erase(*cVert);

			for (auto neighbour : neighbourTriangles[*cVert]) {
				if (
					freeTriangles.find(neighbour) !=
					freeTriangles.end())
				{
					clusterNeighbours.insert(neighbour);
				}
			}
		}

		clusters.push_back(cluster);

		Logger::Verbose() << "Cluster " << clusters.size() <<
			" built. Vertices remaining: " << freeTriangles.size();
	}

	Logger::Verbose() << "Planet: Clusterization end.";




	int tw;
	int th;
	auto td = Loader::LoadImage("Models/Ship/MainBlocks/Wall.png", tw, th);
	_blockTexture = _video->GetTextures()->AddTexture(tw, th, td);


	Logger::Verbose() << "Planet: Texture loaded.";



	glm::vec4 colorMul(0, 0, 0, 1);

	for (auto& cluster : clusters) {
		Loader::VertexData modelData;

		uint32_t nextIndex = 0;
		std::map<Vec3, uint32_t> uniqueVertices;

		for (auto t : cluster) {
			if (uniqueVertices.find(t->P1) == uniqueVertices.end()) {
				uniqueVertices[t->P1] = nextIndex;
				++nextIndex;
				modelData.Vertices.push_back(t->P1);
				modelData.Normals.push_back(
					glm::normalize((glm::vec3)t->P1));
				modelData.TexCoords.push_back({0.55, 0.5});
			}

			if (uniqueVertices.find(t->P2) == uniqueVertices.end()) {
				uniqueVertices[t->P2] = nextIndex;
				++nextIndex;
				modelData.Vertices.push_back(t->P2);
				modelData.Normals.push_back(
					glm::normalize((glm::vec3)t->P2));
				modelData.TexCoords.push_back({0.55, 0.5});
			}

			if (uniqueVertices.find(t->P3) == uniqueVertices.end()) {
				uniqueVertices[t->P3] = nextIndex;
				++nextIndex;
				modelData.Vertices.push_back(t->P3);
				modelData.Normals.push_back(
					glm::normalize((glm::vec3)t->P3));
				modelData.TexCoords.push_back({0.55, 0.5});
			}

			modelData.Indices.push_back(uniqueVertices[t->P1]);
			modelData.Indices.push_back(uniqueVertices[t->P2]);
			modelData.Indices.push_back(uniqueVertices[t->P3]);

			/*modelData.Indices.push_back(uniqueVertices[t->P2]);
			modelData.Indices.push_back(uniqueVertices[t->P1]);
			modelData.Indices.push_back(uniqueVertices[t->P3]);*/
		}

		modelData.Instances = {glm::mat4(1.0)};

		glm::vec3 normal(0, 0, 0);

		for (size_t i = 0; i < modelData.Normals.size(); ++i) {
			normal += modelData.Normals[i];
		}

		normal /= (float)modelData.Normals.size();

		_blockModel.push_back(_video->LoadModel(modelData));

		Segment* segment = new Segment;

		segment->Normal = normal;

		segment->SetObjectVertices(modelData.Vertices);
		segment->SetObjectNormals(modelData.Normals);
		segment->SetObjectIndices(modelData.Indices);

		segment->SetModelExternalMatrix(&_matrix);
		segment->SetModelMatrix(glm::mat4(1.0));

		segment->SetTexture({_blockTexture});
		segment->SetModels({_blockModel.back()});
		segment->SetDrawEnabled(true);
		segment->SetDrawLight(true);

		segment->SetColorMultiplier(colorMul);

		if (colorMul.r < 1) {
			colorMul.r += 0.2;
		} else if (colorMul.g < 1) {
			colorMul.g += 0.2;
			colorMul.r = 0;
		} else if (colorMul.b < 1) {
			colorMul.b += 0.2;
			colorMul.r = 0;
			colorMul.g = 0;
		} else {
			colorMul.r = 0;
			colorMul.g = 0;
			colorMul.b = 0;
		}
		
		_video->RegisterModel(segment);

		_segments.push_back(segment);
	}

	Logger::Verbose() << "Planet: Models loaded.";
}

Planet::~Planet()
{
	for (Segment* segment : _segments) {
		_video->RemoveModel(segment);
		delete segment;
	}

	for (auto model : _blockModel) {
		_video->UnloadModel(model);
	}

	_video->GetTextures()->RemoveTexture(_blockTexture);
}
