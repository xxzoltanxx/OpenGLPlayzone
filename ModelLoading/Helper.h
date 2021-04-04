#pragma once
#include <assimp/scene.h>
struct VertexData
{
	float vertX;
	float vertY;
	float vertZ;

	float normalX;
	float normalY;
	float normalZ;

	float texU;
	float texV;
};

struct Texture
{
	aiTextureType type;
	int id;
	int index;
};
