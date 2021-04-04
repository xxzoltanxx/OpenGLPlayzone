#pragma once
#include "Mesh.h"
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Model : public Drawable
{
public:
	Model(const std::string& filename);

	void draw(Window& window, Shader& shader) override;
	void setRotation(const glm::fquat& rot) { rotation = rot; }
	void setScale(const glm::vec3& scale) { this->scale = scale; }
	void setPosition(glm::vec3 position) { this->position = position; }
private:

	unsigned int loadTexture(const std::string& filename);
	std::vector<Mesh> meshes;
	std::unordered_map<std::string, unsigned int> textures;

	std::string directory;
	glm::vec3 position;
	glm::vec3 scale = glm::vec3(1,1,1);
	glm::fquat rotation = glm::fquat(1,0,0,0);
	//Cache
	static std::unordered_map<std::string, int> textureCache;
};

class Sprite : public Drawable
{
public:
	Sprite(const std::string& filename);
	Sprite(unsigned int texture);

	void draw(Window& window, Shader& shader) override;
	void setRotation(const glm::fquat& rot) { rotation = rot; }
	void setScale(const glm::vec3& scale) { this->scale = scale; }
	void setPosition(glm::vec3 position) { this->position = position; }
private:
	glm::vec3 position = glm::vec3(0,0,0);
	glm::vec3 scale = glm::vec3(1, 1, 1);
	glm::fquat rotation = glm::fquat(1, 0, 0, 0);
	unsigned int textureID;


	static unsigned int VBO;
	static unsigned int VAO;
	static unsigned int EBO;
	static std::unordered_map<std::string, unsigned int> textureCache;

	static float verticesData[20];
	static unsigned int indices[6];
};