#include "planet.h"

#include <vector>
#include <cstdlib>
#include <fstream>

#include "../Logger/logger.h"

static glm::dvec3 VecToGlm(const Math::Vec<3>& vec)
{
	return glm::dvec3(vec[0], vec[1], vec[2]);
}

static Math::Mat<4> GlmToMat(const glm::dmat4& mat)
{
	Math::Mat<4> res;

	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			res[row][col] = mat[col][row];
		}
	}

	return res;
}

struct Vec3
{
	double x;
	double y;
	double z;

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

	Vec3(const glm::dvec3& vec)
	{
		x = vec.x;
		y = vec.y;
		z = vec.z;
	}

	Vec3(const Math::Vec<3>& vec)
	{
		x = vec[0];
		y = vec[1];
		z = vec[2];
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

	operator glm::dvec3() const
	{
		return glm::dvec3(x, y, z);
	}

	operator Math::Vec<3>() const
	{
		return Math::Vec<3>({x, y, z});
	}
};

struct Triangle
{
	Vec3 P1;
	Vec3 P2;
	Vec3 P3;

	Vec3 Center;
	double Radius;

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
		Center = ((glm::dvec3)P1 + (glm::dvec3)P2 + (glm::dvec3)P3) /
			3.0;
		Radius = std::max<double>(std::max<double>(
			glm::length((glm::dvec3)Center - (glm::dvec3)P1),
			glm::length((glm::dvec3)Center - (glm::dvec3)P2)),
			glm::length((glm::dvec3)Center - (glm::dvec3)P3));
	}
};

static Math::Vec<3> AngleToPoint(
	double lat,
	double lon,
	std::map<double, std::map<double, double>>& radius)
{
	double latRad = glm::radians(lat);
	double lonRad = glm::radians(lon);

	Math::Vec<3> dir = {
		sin(lonRad) * cos(latRad),
		cos(lonRad) * cos(latRad),
		sin(latRad)
	};

	dir *= radius[lat][lon];

	return dir;
}

static bool Neighbours(const Triangle& t1, const Triangle& t2)
{
	double dist =
		glm::length((glm::dvec3)t1.Center - (glm::dvec3)t2.Center);

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
	double radius,
	Math::Vec<3> position,
	Video* video,
	PhysicalEngine* physicalEngine)
{
	_video = video;
	_physicalEngine = physicalEngine;

	_position = position;
	_matrix = GlmToMat(glm::translate(glm::dmat4(1.0), VecToGlm(position)));

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

				clusterSizes[cIdx] = clusterSize;

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

			if (it != triangles.end()) {
				Logger::Error() << "Error in cluster reading";
			}
		}
	}

	if (triangles.empty()) {
		std::map<double, std::vector<double>> angles;
		std::map<double, std::map<double, double>> radiuses;

		Logger::Verbose() << "Planet: Building surface.";

		double latStep = 0.05;

		for (
			double angleLat = -90.0 + latStep / 2.0;
			angleLat <= 90.0 - latStep / 2.0;
			angleLat += latStep)
		{
			int lonAngleCount = ceilf(360.0 / latStep *
				fabs(cosf(glm::radians(angleLat))));

			double lonStep = 360.0 / lonAngleCount;

			Logger::Verbose() << "Planet: Lat: " << angleLat <<
				", point count: " << lonAngleCount;

			std::vector<double> dirs;
			std::map<double, double> rad;

			for (
				double angleLon = 0;
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

		double lonBase = angles.begin()->second.front();
		double latBase = angles.begin()->first;

		for (
			auto lonIt = angles.begin()->second.begin() + 1;
			lonIt != angles.begin()->second.end() - 1;
			++lonIt)
		{
			Math::Vec<3> p1 = AngleToPoint(
				latBase,
				lonBase,
				radiuses);

			Math::Vec<3> p2 = AngleToPoint(
				latBase,
				*lonIt,
				radiuses);

			Math::Vec<3> p3 = AngleToPoint(
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

			double lon1 = *lonIt1;
			double lon2 = *lonIt2;

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
					double prevL = *lonIt1;
					++lonIt1;

					Math::Vec<3> p1 = AngleToPoint(
						latIt1->first,
						prevL,
						radiuses);

					Math::Vec<3> p2 = AngleToPoint(
						latIt2->first,
						lon2,
						radiuses);

					Math::Vec<3> p3 = AngleToPoint(
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
					double prevL = *lonIt2;
					++lonIt2;

					Math::Vec<3> p1 = AngleToPoint(
						latIt1->first,
						lon1,
						radiuses);

					Math::Vec<3> p2 = AngleToPoint(
						latIt2->first,
						prevL,
						radiuses);

					Math::Vec<3> p3 = AngleToPoint(
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

			Math::Vec<3> p1 = AngleToPoint(
				latIt1->first,
				lon1,
				radiuses);

			Math::Vec<3> p2 = AngleToPoint(
				latIt2->first,
				lon2,
				radiuses);

			Math::Vec<3> p3 = AngleToPoint(
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
			Math::Vec<3> p1 = AngleToPoint(
				latBase,
				lonBase,
				radiuses);

			Math::Vec<3> p2 = AngleToPoint(
				latBase,
				*lonIt,
				radiuses);

			Math::Vec<3> p3 = AngleToPoint(
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

			double coeff =
				sin((double)index / triangles.size() * M_PI) +
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
				Math::Vec<3> clusterCenter(0.0);

				for (auto vertex : cluster) {
					clusterCenter += vertex->Center;
				}

				clusterCenter /= (double)cluster.size();

				double cDist = radius * 2.0;

				auto cVert = clusterNeighbours.begin();

				auto vert = clusterNeighbours.begin();
				while (vert != clusterNeighbours.end()) {
					double dist =
						(clusterCenter -
						(*vert)->Center).Length();

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

	std::list<Math::Vec<3>> normals;

	for (auto& cluster : clusters) {
		Math::Vec<3> normal(0.0);

		for (auto& triangle : cluster) {
			normal += ((Math::Vec<3>)triangle->P1).Normalize();
			normal += ((Math::Vec<3>)triangle->P2).Normalize();
			normal += ((Math::Vec<3>)triangle->P3).Normalize();
		}

		normal = normal.Normalize();
		normals.push_back(normal);
	}

	bool mergedCluster = true;
	bool clustersChanged = false;

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
				clustersChanged = true;

				auto nearestCluster = clusters.begin();
				auto nearestNormal = normals.begin();

				if (nearestCluster == it) {
					++nearestCluster;
					++nearestNormal;
				}

				auto nCluster = clusters.begin();
				auto nNormal = normals.begin();

				while (nCluster != clusters.end()) {
					if (nNormal == normIt) {
						++nCluster;
						++nNormal;
						continue;
					}

					double angleCos =
						normIt->Dot(*nNormal);

					double angleCosRef = normIt->Dot(
						*nearestNormal);

					if (angleCos > angleCosRef) {
						nearestNormal = nNormal;
						nearestCluster = nCluster;
					}

					++nCluster;
					++nNormal;
				}

				for (auto& t : *it) {
					nearestCluster->push_back(t);
				}

				Math::Vec<3> normal(0.0);

				for (auto& t : *nearestCluster) {
					normal +=
						((Math::Vec<3>)t->P1)
							.Normalize();
					normal +=
						((Math::Vec<3>)t->P2)
							.Normalize();
					normal +=
						((Math::Vec<3>)t->P3)
							.Normalize();
				}

				normal = normal.Normalize();
				*nearestNormal = normal;

				clusters.erase(it);
				normals.erase(normIt);
				break;
			}
		}

		Logger::Verbose() << "Clusters remaining: " <<
			clusters.size();
	}

	if (clustersChanged) {
		std::fstream clusterFile(
			"../cluster_dump.bin",
			std::ios::out | std::ios::binary | std::ios::trunc);

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
	}

	Logger::Verbose() << "Planet: Clusterization end.";

	auto td = Loader::LoadImage("Images/Grass.png");
	_blockTexture = _video->LoadTexture(td);

	Logger::Verbose() << "Planet: Texture loaded.";

	//glm::vec4 colorMul(0, 0, 0, 1);

	for (auto& cluster : clusters) {
		Segment* segment = new Segment;

		uint32_t nextIndex = 0;
		std::map<Vec3, uint32_t> uniqueVertices;

		for (auto t : cluster) {
			if (
				uniqueVertices.find(t->P1) ==
				uniqueVertices.end())
			{
				uniqueVertices[t->P1] = nextIndex;
				++nextIndex;
				segment->Geometry.Vertices.push_back(t->P1);
				segment->Geometry.Normals.push_back(
					((Math::Vec<3>)t->P1).Normalize());
				segment->Geometry.TexCoords.push_back(
					{0.0, 0.0});
				segment->Geometry.MatrixIndices.push_back(0);
			}

			if (
				uniqueVertices.find(t->P2) ==
				uniqueVertices.end())
			{
				uniqueVertices[t->P2] = nextIndex;
				++nextIndex;
				segment->Geometry.Vertices.push_back(t->P2);
				segment->Geometry.Normals.push_back(
					((Math::Vec<3>)t->P2).Normalize());
				segment->Geometry.TexCoords.push_back(
					{0.0, 100.0});
				segment->Geometry.MatrixIndices.push_back(0);
			}

			if (
				uniqueVertices.find(t->P3) ==
				uniqueVertices.end())
			{
				uniqueVertices[t->P3] = nextIndex;
				++nextIndex;
				segment->Geometry.Vertices.push_back(t->P3);
				segment->Geometry.Normals.push_back(
					((Math::Vec<3>)t->P3).Normalize());
				segment->Geometry.TexCoords.push_back(
					{100.0, 100.0});
				segment->Geometry.MatrixIndices.push_back(0);
			}

			segment->Geometry.Indices.push_back(
				uniqueVertices[t->P1]);
			segment->Geometry.Indices.push_back(
				uniqueVertices[t->P2]);
			segment->Geometry.Indices.push_back(
				uniqueVertices[t->P3]);
		}

		segment->Geometry.Instances = {Math::Mat<4>(1.0)};

		Math::Vec<3> normal(0.0);

		for (size_t i = 0; i < segment->Geometry.Normals.size(); ++i) {
			normal += segment->Geometry.Normals[i];
		}

		normal /= (double)segment->Geometry.Normals.size();

		segment->Loaded = false;
		segment->Activated = false;

		segment->Normal = normal.Normalize();

		segment->PhysicalParams.Vertices = segment->Geometry.Vertices;
		segment->PhysicalParams.Normals = segment->Geometry.Normals;
		segment->PhysicalParams.Indices = segment->Geometry.Indices;
		segment->PhysicalParams.ExternalMatrix = &_matrix;
		segment->PhysicalParams.Matrix = Math::Mat<4>(1.0);
		segment->PhysicalParams.Enabled = true;
		segment->PhysicalParams.Mu = 0.3;
		segment->PhysicalParams.Bounciness = 0.1;

		segment->ModelParams.ExternalMatrix = &_matrix;
		segment->ModelParams.Matrix = Math::Mat<4>(1.0);

		segment->TextureParams.SetAll(_blockTexture);
		//segment->TextureParams.IsLight = true;

		/*segment->DrawParams.ColorMultiplier = colorMul;

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
		}*/
		
		_segments.push_back(segment);

		Logger::Verbose() << "Planet: Loaded segment " <<
			_segments.size() << " / " << clusters.size();
	}

	Logger::Verbose() << "Planet: Models loaded.";

	size_t segmentIndex = 0;
	for (Segment* segment : _segments) {
		std::set<Segment*> neighbours;

		for (Segment* seg : _segments) {
			if (seg == segment) {
				continue;
			}

			double angleCos = segment->Normal.Dot(seg->Normal);

			if (angleCos > 0.99) {
				neighbours.insert(seg);
			}
		}

		_segmentNeighbours[segment] = neighbours;
		_subActiveSegments.insert(segment);

		Logger::Verbose() << "Segment " << segmentIndex << " / " <<
			_segments.size();
		++segmentIndex;
	}

	Logger::Verbose() << "Planet: planet built.";
}

Planet::~Planet()
{
	for (Segment* segment : _segments) {
		_video->RemoveModel(segment);
		_physicalEngine->RemoveObject(segment);
		UnloadSegment(segment);
		delete segment;
	}

	_video->UnloadTexture(_blockTexture);
}

void Planet::Update(const Math::Vec<3>& playerCoord)
{
	Math::Vec<3> playerNorm = (playerCoord - _position).Normalize();

	std::set<Segment*> segToDeactivate;
	std::set<Segment*> segToActivate;
	std::set<Segment*> segToRemove;

	for (Segment* segment : _activeSegments) {
		double angleCos = playerNorm.Dot(segment->Normal);

		if (angleCos < 0.985) {
			segToDeactivate.insert(segment);
		}
	}

	for (Segment* segment : _subActiveSegments) {
		double angleCos = playerNorm.Dot(segment->Normal);

		if (angleCos > 0.995) {
			segToActivate.insert(segment);
		} else if (angleCos < 0.975) {
			segToRemove.insert(segment);
		}
	}

	for (Segment* segment : segToRemove) {
		_subActiveSegments.erase(segment);
		UnloadSegment(segment);
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

	//Logger::Verbose() << "Active segments: " << _activeSegments.size();
	//Logger::Verbose() << "Tracked segments: " <<
	//	_activeSegments.size() + _subActiveSegments.size();
}

void Planet::ActivateSegment(Segment* segment)
{
	if (segment->Activated) {
		return;
	}

	LoadSegment(segment);
	segment->DrawParams.Enabled = true;
	_video->RegisterModel(segment);
	_physicalEngine->RegisterObject(segment);
	segment->Activated = true;
}

void Planet::DeactivateSegment(Segment* segment)
{
	if (!segment->Activated) {
		return;
	}

	segment->DrawParams.Enabled = false;
	_video->RemoveModel(segment);
	_physicalEngine->RemoveObject(segment);
	segment->Activated = false;
}

void Planet::LoadSegment(Segment* segment)
{
	if (segment->Loaded) {
		return;
	}

	segment->ModelParams.Model = _video->LoadModel(segment->Geometry, true);
	segment->Loaded = true;
}

void Planet::UnloadSegment(Segment* segment)
{
	if (!segment->Loaded) {
		return;
	}

	DeactivateSegment(segment);

	_video->UnloadModel(segment->ModelParams.Model);
	segment->Loaded = false;
}
