#include "Mesh.h"
#include <glad/glad.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "Shader.h"
#include "Helper.h"
#include <GLFW/glfw3.h>

void Mesh::draw(Window& window, Shader& shader)
{
	//Shader is used in the model, uniforms are set there

	glBindVertexArray(VAO);


	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;

	for (int i = 0; i < textures.size(); ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
		std::string textureToBind;
		if (textures[i].type == aiTextureType_DIFFUSE)
		{
			textureToBind = std::string("material.texture_diffuse") + std::to_string(diffuseNr++);
		}
		else if (textures[i].type == aiTextureType_SPECULAR)
		{
			textureToBind = std::string("material.texture_specular") + std::to_string(specularNr++);
		}
		glUniform1i(glGetUniformLocation(shader.getID(), textureToBind.c_str()), i);
	}

	glUniform1i(glGetUniformLocation(shader.getID(), "shininess"), 32);


	//setting a static directional light for now

	glUniform3f(glGetUniformLocation(shader.getID(), "directionalLight.direction"), sin(glfwGetTime() * 1.5f), -2 , cos(glfwGetTime() * 1.5f));
	glUniform3f(glGetUniformLocation(shader.getID(), "directionalLight.ambient"), 0.2f, 0.2f, 0.2f);
	glUniform3f(glGetUniformLocation(shader.getID(), "directionalLight.diffuse"), 0.5f, 0.5f, 0.5f);
	glUniform3f(glGetUniformLocation(shader.getID(), "directionalLight.specular"), 1, 1, 1);

	glActiveTexture(GL_TEXTURE0);

	glDrawElements(GL_TRIANGLES, facesSize, GL_UNSIGNED_INT, 0);

}

void Mesh::loadToGPU(const std::vector<unsigned int>& indices, const std::vector<VertexData>& vertices, const std::vector<Texture>& textures)
{
	facesSize = indices.size();
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}