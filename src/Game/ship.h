#ifndef _SHIP_H
#define _SHIP_H

#include "../VideoEngine/video.h"
#include "../PhysicalEngine/object.h"
#include "../UniverseEngine/actor.h"

class Block : public Model, public Object
{
public:
	Block(Video* video, const glm::ivec3& position, uint32_t texture);
	~Block();

private:
	Video* _video;
};

class Ship : public Actor
{
public:
	struct BlockCoord
	{
		int32_t X;
		int32_t Y;
		int32_t Z;

		BlockCoord(const glm::ivec3& coord)
		{
			X = coord.x;
			Y = coord.y;
			Z = coord.z;
		}

		bool operator<(const BlockCoord& coord) const
		{
			if (X != coord.X) {
				return X < coord.X;
			}

			if (Y != coord.Y) {
				return Y < coord.Y;
			}

			if (Z != coord.Z) {
				return Z < coord.Z;
			}

			return false;
		}
	};

	Ship(Video* video, uint32_t blockTexture);
	~Ship();

	void Tick() override;

	void InsertBlock(const glm::ivec3& position);
	void RemoveBlock(const glm::ivec3& position);

	void PreviewBlock(const glm::ivec3& position);
	void StopPreview();

	void SaveToFile(std::string file);
	void LoadFromFile(std::string file);

private:
	Video* _video;

	std::map<BlockCoord, glm::mat4> _grid;
	Block* _block;
	Block* _previewBlock;

	uint32_t _blockTexture;

	glm::vec3 _pos;
	glm::vec3 _rotation;
	glm::vec3 _linearSpeed;
	glm::vec3 _angularSpeed;

	void UpdateView();
};

#endif
