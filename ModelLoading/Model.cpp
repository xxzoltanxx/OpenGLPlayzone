#include "Model.h"
#include "Shader.h"
#include <glad/glad.h>
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Window.h"

std::unordered_map<std::string, int> Model::textureCache = std::unordered_map<std::string, int>();

void Model::draw(Window& window, Shader& shader)
{
	shader.use();

	glUniformMatrix4fv(glGetUniformLocation(shader.getID(), "view"), 1, GL_FALSE, glm::value_ptr(window.getView()));
	glUniformMatrix4fv(glGetUniformLocation(shader.getID(), "projection"), 1, GL_FALSE, glm::value_ptr(window.getProjection()));

	glm::mat4 rotationMatrix = glm::toMat4(rotation);

	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

	glm::mat4 combined = translationMatrix * rotationMatrix * scaleMatrix;

	glUniformMatrix4fv(glGetUniformLocation(shader.getID(), "model"), 1, GL_FALSE, glm::value_ptr(combined));
	glUniform3f(glGetUniformLocation(shader.getID(), "viewPos"), window.getCameraPosition().x, window.getCameraPosition().y, window.getCameraPosition().z);

	for (auto& mesh : meshes)
	{
		mesh.draw(window, shader);
	}
}

unsigned int Model::loadTexture(const std::string& filename)
{
	if (textureCache.find(filename) != textureCache.end())
	{
		return textureCache[filename];
	}
	int width, height, channels;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);

	unsigned int texID;
	glGenTextures(1, &texID);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	textureCache.insert(std::make_pair(filename, texID));
	return texID;
}

Model::Model(const std::string& filename)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filename.c_str(), aiProcess_Triangulate);
	directory = directory.substr(0, filename.find_last_of('/'));

	for (int i = 0; i < scene->mNumMeshes; ++i)
	{

		std::vector<Texture> texturesToInsert;
		std::vector<VertexData> vertices;
		std::vector<unsigned int> indices;

		aiMesh* mesh = scene->mMeshes[i];
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		aiString fileName;
		Texture textureToInsert;
		for (int j = 0; j < material->GetTextureCount(aiTextureType_DIFFUSE); ++j)
		{
			textureToInsert.type = aiTextureType_DIFFUSE;
			textureToInsert.index = j;
			material->GetTexture(aiTextureType_DIFFUSE, j, &fileName);
			unsigned int id = loadTexture(std::string(fileName.C_Str()));
			textureToInsert.id = id;
			texturesToInsert.push_back(textureToInsert);
		}
		for (int j = 0; j < material->GetTextureCount(aiTextureType_SPECULAR); ++j)
		{
			textureToInsert.type = aiTextureType_SPECULAR;
			textureToInsert.index = j;
			material->GetTexture(aiTextureType_SPECULAR, j, &fileName);
			unsigned int id = loadTexture(std::string(fileName.C_Str()));
			textureToInsert.id = id;
			texturesToInsert.push_back(textureToInsert);
		}

		VertexData data;
		for (int j = 0; j < mesh->mNumVertices; ++j)
		{
			data.vertX = mesh->mVertices[j].x;
			data.vertY = mesh->mVertices[j].y;
			data.vertZ = mesh->mVertices[j].z;

			data.normalX = mesh->mNormals[j].x;
			data.normalY = mesh->mNormals[j].y;
			data.normalZ = mesh->mNormals[j].z;

			data.texU = mesh->mTextureCoords[0][j].x;
			data.texV = mesh->mTextureCoords[0][j].y;

			vertices.push_back(data);
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace& face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		meshes.push_back(Mesh(indices, vertices, texturesToInsert));
	}
}

unsigned int Sprite::VBO = 0;
unsigned int Sprite::VAO = 0;
unsigned int Sprite::EBO = 0;

std::unordered_map<std::string, unsigned int> Sprite::textureCache = std::unordered_map<std::string, unsigned int>();

unsigned int Sprite::indices[] = {
	0, 1, 3, // first triangle
	1, 2, 3  // second triangle
};

void Sprite::draw(Window& window, Shader& shader)
{
	shader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glUniform1i(glGetUniformLocation(shader.getID(), "textureApply"), 0);
	glUniformMatrix4fv(glGetUniformLocation(shader.getID(), "view"), 1, GL_FALSE, glm::value_ptr(window.getView()));
	glUniformMatrix4fv(glGetUniformLocation(shader.getID(), "projection"), 1, GL_FALSE, glm::value_ptr(window.getProjection()));

	glm::mat4 translationn = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 scalee = glm::scale(glm::mat4(1.0f), scale);
	glm::mat4 rotationn = glm::toMat4(rotation);

	glUniformMatrix4fv(glGetUniformLocation(shader.getID(), "model"), 1, GL_FALSE, glm::value_ptr(translationn * scalee * rotationn));
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

}
float Sprite::verticesData[] = {
	 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,   // top right
	 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,   // bottom right
	-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,   // bottom left
	-0.5f,  0.5f, 0.0f, 0.0f, 1.0f    // top left 
};

Sprite::Sprite(unsigned int texture)
{
	if (VAO == 0)
	{
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verticesData), verticesData, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	textureID = texture;
}

Sprite::Sprite(const std::string& filename)
{
	if (VAO == 0)
	{
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verticesData), verticesData, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	if (textureCache.find(filename) != textureCache.end())
	{
		textureID = textureCache[filename];
	}
	else
	{
		glGenTextures(1, &textureID);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);

		int width, height, channels;
		unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		textureCache.insert(std::make_pair(filename, textureID));
	}
}