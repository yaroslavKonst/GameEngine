#include "planet.h"

#include "../Logger/logger.h"

Planet::Planet(float radius, Video* video)
{
	_video = video;

	_matrix = glm::translate(glm::mat4(1.0), glm::vec3(0, 0, 2));

	std::list<glm::vec3> directions;

	Logger::Verbose() << "Planet: Building surface.";

	float latStep = 2;

	for (
		float angleLat = -90 + latStep;
		angleLat < 90;
		angleLat += latStep)
	{
		int lonAngleCount = ceilf(360.0 / latStep *
			fabs(cosf(glm::radians(angleLat))));

		float lonStep = 360.0 / lonAngleCount;

		Logger::Verbose() << "Planet: Lat: " << angleLat <<
			", point count: " << lonAngleCount;

		for (float angleLon = 0; angleLon < 360; angleLon += lonStep) {
			float latRad = glm::radians(angleLat);
			float lonRad = glm::radians(angleLon);

			glm::vec3 dir = {
				sinf(lonRad) * cosf(latRad),
				cosf(lonRad) * cosf(latRad),
				sinf(latRad)
			};

			dir *= radius;

			directions.push_back(dir);
		}
	}

	Logger::Verbose() << "Planet: Surface built.";

	std::vector<glm::mat4> instances;

	size_t clusterSizeLimit = 100;

	std::list<std::list<glm::vec3>> vertexClusters;

	Logger::Verbose() << "Planet: Clusterization.";

	while (!directions.empty()) {
		std::list<glm::vec3> cluster;
		cluster.push_back(directions.front());
		directions.pop_front();

		while (!directions.empty() &&
			cluster.size() < clusterSizeLimit)
		{
			float cDist = radius * 2;

			auto cVert = directions.begin();

			auto vert = directions.begin();
			while (vert != directions.end()) {
				float dist = 0;

				for (auto& vertex : cluster) {
					dist += glm::length(vertex - *vert);
				}

				dist /= cluster.size();

				if (dist < cDist) {
					cDist = dist;
					cVert = vert;
				}

				++vert;
			}

			cluster.push_back(*cVert);
			directions.erase(cVert);
		}

		vertexClusters.push_back(cluster);

		Logger::Verbose() << "Cluster " << vertexClusters.size() <<
			" built. Vertices remaining: " << directions.size();
	}

	Logger::Verbose() << "Planet: Clusterization end.";

	int tw;
	int th;
	auto td = Loader::LoadImage("Models/Ship/MainBlocks/Wall.png", tw, th);
	_blockTexture = _video->GetTextures()->AddTexture(tw, th, td);

	auto model = Loader::LoadModel("Models/Ship/MainBlocks/Wall.obj");
	for (auto& v : model.Vertices) {
		v *= 0.01;
		v.z /= 3.0;
	}

	glm::vec4 colorMul(0, 0, 0, 1);

	for (auto& cluster : vertexClusters) {
		instances.clear();

		for (glm::vec3& dir : cluster) {
			instances.push_back(glm::translate(
				glm::mat4(1.0),
				dir));
		}

		model.Instances = instances;
		_blockModel.push_back(_video->LoadModel(model));

		Segment* segment = new Segment;

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
		}
		
		_video->RegisterModel(segment);

		_segments.push_back(segment);
	}
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
