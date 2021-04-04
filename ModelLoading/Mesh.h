#pragma once
#include "Drawable.h"
#include "Helper.h"
#include <vector>

class Mesh : public Drawable
{
public:
	Mesh(const std::vector<unsigned int>& indices, const std::vector<VertexData>& vertices, const std::vector<Texture>& textures)
	{
		this->textures = textures;
		loadToGPU(indices, vertices, textures);
	}

	void draw(Window& window, Shader& shader) override;
private:
	void loadToGPU(const std::vector<unsigned int>& indices, const std::vector<VertexData>& vertices, const std::vector<Texture>& textures);
	unsigned int VAO, VBO, EBO;
	unsigned int facesSize;

	std::vector<Texture> textures;
};