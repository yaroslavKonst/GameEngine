#include "planet.h"

#include <vector>
#include <cstdlib>
#include <fstream>

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

static glm::vec3 AngleToPoint(
	float lat,
	float lon,
	std::map<float, std::map<float, float>>& radius)
{
	float latRad = glm::radians(lat);
	float lonRad = glm::radians(lon);

	glm::vec3 dir = {
		sinf(lonRad) * cosf(latRad),
		cosf(lonRad) * cosf(latRad),
		sinf(latRad)
	};

	dir *= radius[lat][lon];

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

Planet::Planet(
	float radius,
	glm::vec3 position,
	Video* video,
	CollisionEngine* collisionEngine)
{
	_video = video;
	_collisionEngine = collisionEngine;

	_position = position;
	_matrix = glm::translate(glm::mat4(1.0), position);

	size_t clusterSizeLimit = 1000;

	std::vector<Triangle> triangles;
	typedef std::vector<Triangle>::iterator TrianglePtr;
	std::list<std::list<TrianglePtr>> clusters;

	{
		std::fstream inClusters(
			"../cluster_dump.bin",
			std::ios::in | std::ios::binary);

		if (inClusters.is_open()) {
			size_t clusterCount;
			inClusters.read(
				(char*)&clusterCount,
				sizeof(clusterCount));
			std::vector<size_t> clusterSizes(clusterCount);

			for (size_t cIdx = 0; cIdx < clusterCount; ++cIdx) {
				size_t clusterSize;
				inClusters.read(
					(char*)&clusterSize,
					sizeof(clusterSize));

				clusterSizes.push_back(clusterSize);

				for (
					size_t tIdx = 0;
					tIdx < clusterSize;
					++tIdx)
				{
					Vec3 p1;
					Vec3 p2;
					Vec3 p3;

					inClusters.read((char*)&p1, sizeof(p1));
					inClusters.read((char*)&p2, sizeof(p2));
					inClusters.read((char*)&p3, sizeof(p3));

					Triangle t;
					t.P1 = p1;
					t.P2 = p2;
					t.P3 = p3;

					triangles.push_back(t);
				}
			}

			auto it = triangles.begin();

			for (size_t cIdx = 0; cIdx < clusterCount; ++cIdx) {
				std::list<TrianglePtr> cluster;

				for (
					size_t tIdx = 0;
					tIdx < clusterSizes[cIdx];
					++tIdx)
				{
					cluster.push_back(it);
					++it;
				}

				clusters.push_back(cluster);
			}
		}
	}

	if (triangles.empty()) {
		std::map<float, std::vector<float>> angles;
		std::map<float, std::map<float, float>> radiuses;

		Logger::Verbose() << "Planet: Building surface.";

		float latStep = 0.05;

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
			std::map<float, float> rad;

			for (
				float angleLon = 0;
				angleLon < 360;
				angleLon += lonStep)
			{
				dirs.push_back(angleLon);
				rad[angleLon] = radius;
			}

			angles[angleLat] = dirs;
			radiuses[angleLat] = rad;
		}

		Logger::Verbose() << "Planet: Vertices built.";

		size_t maxRowSize = 0;

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
				radiuses);

			glm::vec3 p2 = AngleToPoint(
				latBase,
				*lonIt,
				radiuses);

			glm::vec3 p3 = AngleToPoint(
				latBase,
				*(lonIt + 1),
				radiuses);

			Triangle t;
			t.P1 = p1;
			t.P2 = p2;
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

			size_t rowSize = 0;

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
						radiuses);

					glm::vec3 p2 = AngleToPoint(
						latIt2->first,
						lon2,
						radiuses);

					glm::vec3 p3 = AngleToPoint(
						latIt1->first,
						lon1,
						radiuses);

					lon1 = prevL;

					Triangle t;
					t.P1 = p2;
					t.P2 = p1;
					t.P3 = p3;

					if (t.NonRedundant()) {
						t.SetParams();
						triangles.push_back(t);
						rowSize += 1;
					}
				} else {
					float prevL = *lonIt2;
					++lonIt2;

					glm::vec3 p1 = AngleToPoint(
						latIt1->first,
						lon1,
						radiuses);

					glm::vec3 p2 = AngleToPoint(
						latIt2->first,
						prevL,
						radiuses);

					glm::vec3 p3 = AngleToPoint(
						latIt2->first,
						lon2,
						radiuses);

					lon2 = prevL;

					Triangle t;
					t.P1 = p2;
					t.P2 = p1;
					t.P3 = p3;

					if (t.NonRedundant()) {
						t.SetParams();
						triangles.push_back(t);
						rowSize += 1;
					}
				}
			}

			glm::vec3 p1 = AngleToPoint(
				latIt1->first,
				lon1,
				radiuses);

			glm::vec3 p2 = AngleToPoint(
				latIt2->first,
				lon2,
				radiuses);

			glm::vec3 p3 = AngleToPoint(
				latIt1->first,
				latIt1->second.front(),
				radiuses);

			Triangle t;
			t.P1 = p1;
			t.P2 = p2;
			t.P3 = p3;

			if (t.NonRedundant()) {
				t.SetParams();
				triangles.push_back(t);
				rowSize += 1;
			}

			p1 = AngleToPoint(
				latIt2->first,
				lon2,
				radiuses);

			p2 = AngleToPoint(
				latIt2->first,
				latIt2->second.front(),
				radiuses);

			p3 = AngleToPoint(
				latIt1->first,
				latIt1->second.front(),
				radiuses);

			t.P1 = p1;
			t.P2 = p2;
			t.P3 = p3;

			if (t.NonRedundant()) {
				t.SetParams();
				triangles.push_back(t);
				rowSize += 1;
			}

			rowSize += 5;

			if (maxRowSize < rowSize) {
				maxRowSize = rowSize;
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
				radiuses);

			glm::vec3 p2 = AngleToPoint(
				latBase,
				*lonIt,
				radiuses);

			glm::vec3 p3 = AngleToPoint(
				latBase,
				*(lonIt + 1),
				radiuses);

			Triangle t;
			t.P1 = p2;
			t.P2 = p1;
			t.P3 = p3;

			if (t.NonRedundant()) {
				t.SetParams();
				triangles.push_back(t);
			}
		}

		Logger::Verbose() << "Planet: Triangle count: " <<
			triangles.size();

		Logger::Verbose() << "Planet: Neighbour map building.";

		std::map<TrianglePtr, std::set<TrianglePtr>> neighbourTriangles;

		for (
			auto iter = triangles.begin();
			iter != triangles.end();
			++iter)
		{
			neighbourTriangles[iter] = std::set<TrianglePtr>();
		}

		size_t index = 0;
		for (
			auto iter = triangles.begin();
			iter != triangles.end();
			++iter)
		{
			size_t rowSize = 0;

			float coeff =
				sin((float)index / triangles.size() * M_PI) +
				0.2;

			size_t rowSizeLim = maxRowSize;// * coeff;

			if (index % 1000 == 0) {
				coeff = round(coeff * 1000) / 1000;

				Logger::Verbose() <<
					"Planet: Neighbour map: " <<
					index << " / " << triangles.size() <<
					" " << coeff;
			}

			if (rowSizeLim > maxRowSize) {
				rowSizeLim = maxRowSize;
			}

			for (
				auto it = iter + 1;
				it != triangles.end() && rowSize < rowSizeLim;
				++it, ++rowSize)
			{
				if (Neighbours(*iter, *it)) {
					neighbourTriangles[iter].insert(it);
					neighbourTriangles[it].insert(iter);
				}
			}

			++index;
		}

		Logger::Verbose() << "Planet: Neighbour map built.";

		Logger::Verbose() << "Planet: Clusterization.";

		std::set<TrianglePtr> freeTriangles;

		for (
			auto iter = triangles.begin();
			iter != triangles.end();
			++iter)
		{
			freeTriangles.insert(iter);
		}

		while (!freeTriangles.empty()) {
			std::list<TrianglePtr> cluster;
			cluster.push_back(*freeTriangles.begin());
			freeTriangles.erase(cluster.front());

			std::set<TrianglePtr> clusterNeighbours;

			for (
				auto neighbour :
				neighbourTriangles[cluster.front()])
			{
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
					clusterCenter +=
						(glm::vec3)vertex->Center;
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

				for (
					auto neighbour :
					neighbourTriangles[*cVert])
				{
					if (
						freeTriangles.find(neighbour) !=
						freeTriangles.end())
					{
						clusterNeighbours.insert(
							neighbour);
					}
				}
			}

			clusters.push_back(cluster);

			Logger::Verbose() << "Cluster " << clusters.size() <<
				" built. Vertices remaining: " <<
				freeTriangles.size();
		}

		Logger::Verbose() << "Writing cluster dump file";

		std::fstream clusterFile(
			"../cluster_dump.bin",
			std::ios::out | std::ios::binary);

		size_t clusterCount = clusters.size();

		clusterFile.write((char*)&clusterCount, sizeof(clusterCount));

		for (auto& cluster : clusters) {
			size_t clusterSize = cluster.size();
			clusterFile.write(
				(char*)&clusterSize,
				sizeof(clusterSize));

			for (auto& t : cluster) {
				Vec3 p1 = t->P1;
				Vec3 p2 = t->P2;
				Vec3 p3 = t->P3;

				clusterFile.write((char*)&p1, sizeof(p1));
				clusterFile.write((char*)&p2, sizeof(p2));
				clusterFile.write((char*)&p3, sizeof(p3));
			}
		}

		clusterFile.close();

		Logger::Verbose() << "Cluster dump file is written";
	}

	std::list<glm::vec3> normals;

	for (auto& cluster : clusters) {
		glm::vec3 normal(0, 0, 0);

		for (auto& triangle : cluster) {
			normal += glm::normalize((glm::vec3)triangle->P1);
			normal += glm::normalize((glm::vec3)triangle->P2);
			normal += glm::normalize((glm::vec3)triangle->P3);
		}

		normal /= cluster.size() * 3;
		normal = glm::normalize(normal);
		normals.push_back(normal);
	}

	bool mergedCluster = true;

	while (mergedCluster) {
		mergedCluster = false;

		auto normIt = normals.begin();
		for (
			auto it = clusters.begin();
			it != clusters.end();
			++it, ++normIt)
		{
			if (it->size() < clusterSizeLimit * 0.1) {
				mergedCluster = true;

				Logger::Verbose() << "Found small cluster";

				auto nearestCluster = clusters.begin();
				auto nearestNormal = normals.begin();

				auto nCluster = clusters.begin();
				auto nNormal = normals.begin();

				Logger::Verbose() <<
					"Looking for nearest cluster";

				while (nCluster != clusters.end()) {
					if (*nNormal == *normIt) {
						++nCluster;
						++nNormal;
						continue;
					}

					float angleCos =
						glm::dot(*normIt, *nNormal);

					float angleCosRef = glm::dot(
						*normIt,
						*nearestNormal);

					if (angleCos > angleCosRef) {
						nearestNormal = nNormal;
						nearestCluster = nCluster;
					}

					++nCluster;
					++nNormal;
				}

				Logger::Verbose() << "Merging clusters";

				for (auto& t : *it) {
					nearestCluster->push_back(t);
				}

				glm::vec3 normal(0, 0, 0);

				for (auto& t : *nearestCluster) {
					normal += glm::normalize(
						(glm::vec3)t->P1);
					normal += glm::normalize(
						(glm::vec3)t->P2);
					normal += glm::normalize(
						(glm::vec3)t->P3);
				}

				normal /= nearestCluster->size() * 3;
				normal = glm::normalize(normal);
				*nearestNormal = normal;

				clusters.erase(it);
				break;
			}
		}

		Logger::Verbose() << "Clusters remaining: " <<
			clusters.size();
	}

	Logger::Verbose() << "Planet: Clusterization end.";

	auto td = Loader::LoadImage("Images/White.png");
	_blockTexture = _video->GetTextures()->AddTexture(td);

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
				modelData.TexCoords.push_back({0.0, 0.0});
			}

			if (uniqueVertices.find(t->P2) == uniqueVertices.end()) {
				uniqueVertices[t->P2] = nextIndex;
				++nextIndex;
				modelData.Vertices.push_back(t->P2);
				modelData.Normals.push_back(
					glm::normalize((glm::vec3)t->P2));
				modelData.TexCoords.push_back({0.0, 100.0});
			}

			if (uniqueVertices.find(t->P3) == uniqueVertices.end()) {
				uniqueVertices[t->P3] = nextIndex;
				++nextIndex;
				modelData.Vertices.push_back(t->P3);
				modelData.Normals.push_back(
					glm::normalize((glm::vec3)t->P3));
				modelData.TexCoords.push_back({100.0, 500.0});
			}

			modelData.Indices.push_back(uniqueVertices[t->P1]);
			modelData.Indices.push_back(uniqueVertices[t->P2]);
			modelData.Indices.push_back(uniqueVertices[t->P3]);
		}

		modelData.Instances = {glm::mat4(1.0)};

		glm::vec3 normal(0, 0, 0);

		for (size_t i = 0; i < modelData.Normals.size(); ++i) {
			normal += modelData.Normals[i];
		}

		normal /= (float)modelData.Normals.size();

		_blockModel.push_back(_video->LoadModel(modelData));

		Segment* segment = new Segment;

		segment->Normal = glm::normalize(normal);

		segment->SetObjectVertices(modelData.Vertices);
		segment->SetObjectNormals(modelData.Normals);
		segment->SetObjectIndices(modelData.Indices);
		segment->SetObjectExternalMatrix(&_matrix);
		segment->SetObjectMatrix(glm::mat4(1.0));

		segment->SetModelExternalMatrix(&_matrix);
		segment->SetModelMatrix(glm::mat4(1.0));

		segment->SetTexture({_blockTexture});
		segment->SetModels({_blockModel.back()});
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
		
		_segments.push_back(segment);

		Logger::Verbose() << "Planet: Loaded segment " <<
			_segments.size() << " / " << clusters.size();
	}

	Logger::Verbose() << "Planet: Models loaded.";

	for (Segment* segment : _segments) {
		std::set<Segment*> neighbours;

		for (Segment* seg : _segments) {
			if (seg == segment) {
				continue;
			}

			float angleCos = glm::dot(segment->Normal, seg->Normal);

			if (angleCos > 0.97) {
				neighbours.insert(seg);
			}
		}

		_segmentNeighbours[segment] = neighbours;
		_subActiveSegments.insert(segment);
	}
}

Planet::~Planet()
{
	for (Segment* segment : _segments) {
		_video->RemoveModel(segment);
		_collisionEngine->RemoveObject(segment);
		delete segment;
	}

	for (auto model : _blockModel) {
		_video->UnloadModel(model);
	}

	_video->GetTextures()->RemoveTexture(_blockTexture);
}

void Planet::Update(glm::vec3 playerCoord)
{
	glm::vec3 playerNorm = glm::normalize(playerCoord - _position);

	std::set<Segment*> segToDeactivate;
	std::set<Segment*> segToActivate;
	std::set<Segment*> segToRemove;

	for (Segment* segment : _activeSegments) {
		float angleCos = glm::dot(playerNorm, segment->Normal);

		if (angleCos < 0.985) {
			segToDeactivate.insert(segment);
		}
	}

	for (Segment* segment : _subActiveSegments) {
		float angleCos = glm::dot(playerNorm, segment->Normal);

		if (angleCos > 0.995) {
			segToActivate.insert(segment);
		} else if (angleCos < 0.975) {
			segToRemove.insert(segment);
		}
	}

	for (Segment* segment : segToRemove) {
		_subActiveSegments.erase(segment);
	}

	for (Segment* segment : segToActivate) {
		ActivateSegment(segment);

		for (Segment* seg : _segmentNeighbours[segment]) {
			bool addSegment =
				_activeSegments.find(seg) ==
				_activeSegments.end() &&
				_subActiveSegments.find(seg) ==
				_subActiveSegments.end();

			if (addSegment) {
				_subActiveSegments.insert(seg);
			}
		}

		_activeSegments.insert(segment);
		_subActiveSegments.erase(segment);
	}

	for (Segment* segment : segToDeactivate) {
		DeactivateSegment(segment);
		_activeSegments.erase(segment);
		_subActiveSegments.insert(segment);
	}

	Logger::Verbose() << "Active segments: " << _activeSegments.size();
	Logger::Verbose() << "Tracked segments: " <<
		_activeSegments.size() + _subActiveSegments.size();
}

void Planet::ActivateSegment(Segment* segment)
{
	segment->SetDrawEnabled(true);
	_video->RegisterModel(segment);
	_collisionEngine->RegisterObject(segment);
}

void Planet::DeactivateSegment(Segment* segment)
{
	segment->SetDrawEnabled(false);
	_video->RemoveModel(segment);
	_collisionEngine->RemoveObject(segment);
}
