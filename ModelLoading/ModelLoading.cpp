// ModelLoading.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <glad/glad.h>
#include "Window.h"
#include "Model.h"

class Window;

const char* vertexShaderS = R"(
	#version 330 core
	layout (location = 0) in vec3 pos;
	layout (location = 1) in vec3 normal;
	layout (location = 2) in vec2 uvCord;

	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;

	out vec3 normala;
	out vec2 uv;
	out vec3 posOut;
	
	void main()
	{
		posOut = pos;
		gl_Position = projection * view * model * vec4(pos, 1.0f);
		normala = normalize(mat3(transpose(inverse(model))) * normal);
		uv = uvCord;
	}
)";

const char* outlineShaderSVert = R"(
	#version 330 core
	layout (location = 0) in vec3 pos;
	layout (location = 1) in vec3 normal;
	layout (location = 2) in vec2 uvCord;

	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;

	void main()
	{
		gl_Position = projection * view * model * vec4(pos + (normal * 0.05f), 1.0f);
	}
)";

const char* fragmentShaderS = R"(
	#version 330 core
	in vec3 normala;
	in vec2 uv;
	in vec3 posOut;
	struct Material
	{
		sampler2D texture_diffuse1;
		sampler2D texture_diffuse2;
		sampler2D texture_diffuse3;

		sampler2D texture_specular1;
		sampler2D texture_specular2;
		sampler2D texture_specular3;
		
		float shininess;
	};

	struct DirectionalLight
	{
		vec3 direction;
		vec3 ambient;
		vec3 diffuse;
		vec3 specular;
	};
	uniform mat4 model;
	uniform vec3 viewPos;
	uniform Material material;
	uniform DirectionalLight directionalLight;

	out vec4 finalColor;

	void main()
	{
		vec4 ambient = vec4(texture(material.texture_diffuse1, uv) * vec4(directionalLight.ambient, 1.0));
		vec4 diffuse = vec4(max(dot(normala, -normalize(directionalLight.direction)),0.0) * texture(material.texture_diffuse1, uv));
		vec3 reflected = normalize(reflect(-directionalLight.direction, normala));
		vec4 specular = vec4(pow(max(dot(reflected, normalize(viewPos - vec3((model * vec4(posOut, 1.0)).xyz))), 0.0), 256) * texture(material.texture_specular1, uv));
		
		finalColor = ambient + diffuse + specular;
	}
)";

const char* outlineShader = R"(
	#version 330 core
	out vec4 col;
	void main()
	{
		col = vec4(0.2,0.5,0.7,1.0);
	}
)";

const char* spriteShader = R"(
	#version 330 core
	layout (location = 0) in vec3 pos;
	layout (location = 1) in vec2 texCoord;
	
	uniform mat4 view;
	uniform mat4 projection;
	uniform mat4 model;

	out vec2 tex;

	void main()
	{
		gl_Position = projection * view * model * vec4(pos, 1.0f);
		tex = texCoord;
	}
)";

const char* spriteFragShader = R"(
	#version 330 core
	in vec2 tex;
	uniform sampler2D textureApply;

	out vec4 col;
	void main()
	{
		col = texture(textureApply, tex);
		if (col.a < 0.05f)
		{
			discard;
		}
	}
)";
const char* GaussianBlurShader = R"(
	#version 330 core
	in vec2 tex;
	uniform sampler2D textureApply;

	out vec4 col;
	void main()
	{

	const float offset = 1.0 / 300.0;  

    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right    
    );
	    float kernel[9] = float[](
        1/16.f, 2/16.f, 1/16.f,
        2/16.f,  4/16.f, 2/16.f,
        1/16.f, 2/16.f, 1/16.f
    );
	vec3 sampleTex[9];
	for (int i = 0; i < 9; ++i)
	{
		sampleTex[i] = vec3(texture(textureApply, tex + offsets[i]));
	}
	col = vec4(0.0f);
	for (int i = 0; i < 9; ++i)
	{
		col += vec4(sampleTex[i] * kernel[i],0.0f);
	}
	col = vec4(col.rgb, 1.0f);

	//Just grayscale
	col = texture(textureApply, tex);
	float average = 0.2126 * col.r + 0.7152 * col.g + 0.0722 * col.b;
	//col = vec4(average,average,average,1.0f);

	}
)";

const char* waterShaderVert = R"(
	#version 330 core
	layout (location = 0) in vec2 thisPos;
	layout (location = 1) in vec2 one;
	layout (location = 2) in vec2 two;

	out vec3 normala;
	out vec3 posOut;
	uniform float time;
	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;

	void main()
	{
		float z = sin(time * ((thisPos.x + thisPos.y))) / 5.0f;
		float zOne = sin(time * ((one.x + one.y))) / 5.0f;
		float zTwo = sin(time * ((two.x + two.y))) / 5.0f;
		vec3 position = vec3(thisPos.x, thisPos.y, z);
		vec3 crossed = cross(vec3(one - thisPos,zOne - z), vec3(two - thisPos,zTwo - z));
		normala = normalize(mat3(transpose(inverse(model))) * -crossed);
		posOut = position;
		gl_Position = projection * view * model * vec4(position,1.0f);
	}
)";

const char* waterShaderFrag = R"(
	#version 330 core
	out vec4 col;
	in vec3 normala;
	in vec3 posOut;
	struct DirectionalLight
	{
		vec3 direction;
		vec3 ambient;
		vec3 diffuse;
		vec3 specular;
	};

	uniform DirectionalLight directionalLight;
	uniform mat4 model;
	uniform samplerCube cubeMap;
	uniform vec3 viewPos;

	void main()
	{
		col = mix(vec4(0.1,0.5,0.5,1.0f),texture(cubeMap, reflect(normalize(viewPos - vec3(model * vec4(posOut, 1.0))), normala)), 0.7f);
		vec4 ambient = col * vec4(directionalLight.ambient, 1.0f);
		vec4 diffuse = vec4(max(dot(normala, -normalize(directionalLight.direction)),0.0) * col);
		vec3 reflected = normalize(reflect(-directionalLight.direction, normala));
		vec4 specular = vec4(pow(max(dot(reflected, normalize(viewPos - vec3((model * vec4(posOut, 1.0)).xyz))), 0.0), 32) * col);
		col = ambient + diffuse + specular;
	}
)";

const char* cubemapVertS = R"(
	#version 330 core
	layout (location = 0) in vec3 pos;
	uniform mat4 projection;
	uniform mat4 view;
	
	out vec3 TexCoord;
	void main()
	{
		TexCoord = pos;
		vec4 ayy = projection * mat4(mat3(view)) * vec4(pos,1.0f);
		gl_Position = ayy.xyww;
	}
)";

const char* cubemapFragS = R"(
	#version 330 core
	in vec3 TexCoord;
	uniform samplerCube skybox;
	out vec4 col;
	void main()
	{
		col = texture(skybox, TexCoord);
	}
)";

struct WaterTriData
{
	float thisX;
	float thisY;

	float oneX;
	float oneY;

	float twoX;
	float twoY;
};

class SkyBox : public Drawable
{
public:
	SkyBox(const std::vector<std::string>& faces);
	void draw(Window& window, Shader& shader) override;
	int getTexture() const { return texture; }
private:
	unsigned int VBO;
	unsigned int VAO;
	unsigned int texture;

	static float vertices[];
};

class WaterBody : public Drawable
{
public:
	WaterBody(float width, float height, int nrPerAxis, SkyBox& box);
	void draw(Window& win, Shader& shader) override;
	void setRotation(const glm::fquat& rot) { rotation = rot; }
	void setScale(const glm::vec3& scale) { this->scale = scale; }
	void setPosition(glm::vec3 position) { this->position = position; }
	glm::fquat getRotation() const { return rotation; }
private:
	std::vector<WaterTriData> getVertices(float width, float height, int nrPerAxis = 10);
	unsigned int VAO;
	unsigned int VBO;
	unsigned int skyboxTexture;

	glm::vec3 position = glm::vec3(0, 0, 0);
	glm::vec3 scale = glm::vec3(1, 1, 1);
	glm::fquat rotation = glm::fquat(1, 0, 0, 0);

	int verticesNum = 0;
};

#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

void WaterBody::draw(Window& window, Shader& shader)
{
	shader.use();

	glm::mat4 projection = window.getProjection();
	glm::mat4 view = window.getView();

	glm::mat4 translationn = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 scalee = glm::scale(glm::mat4(1.0f), scale);
	glm::mat4 rotationn = glm::toMat4(rotation);

	glUniformMatrix4fv(glGetUniformLocation(shader.getID(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(glGetUniformLocation(shader.getID(), "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader.getID(), "model"), 1, GL_FALSE, glm::value_ptr(translationn * rotationn * scalee));
	glUniform3f(glGetUniformLocation(shader.getID(), "viewPos"), window.getCameraPosition().x, window.getCameraPosition().y, window.getCameraPosition().z);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	//setting a static directional light for now

	glUniform1i(glGetUniformLocation(shader.getID(), "cubeMap"), 1);

	glUniform3f(glGetUniformLocation(shader.getID(), "directionalLight.direction"), sin(1.5f), -2, cos(1.5f));
	glUniform3f(glGetUniformLocation(shader.getID(), "directionalLight.ambient"), 0.2f, 0.2f, 0.2f);
	glUniform3f(glGetUniformLocation(shader.getID(), "directionalLight.diffuse"), 0.5f, 0.5f, 0.5f);
	glUniform3f(glGetUniformLocation(shader.getID(), "directionalLight.specular"), 1, 1, 1);

	glUniform1f(glGetUniformLocation(shader.getID(), "time"), glfwGetTime());
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, verticesNum);
}

WaterBody::WaterBody(float width, float height, int nrPerAxis, SkyBox& skyboxTexture)
{
	this->skyboxTexture = skyboxTexture.getTexture();
	std::vector<WaterTriData> vertices = getVertices(width, height, nrPerAxis);
	verticesNum = vertices.size();

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(WaterTriData), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(WaterTriData), (void*)(sizeof(float) * 2));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(WaterTriData), (void*)(sizeof(float) * 4));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
}

std::vector<WaterTriData> WaterBody::getVertices(float width, float height, int nrPerAxis)
{
	std::vector<WaterTriData> vertices;
	float xStep = width / nrPerAxis;
	float yStep = height / nrPerAxis;
	for (int i = 0; i < nrPerAxis; ++i)
		for (int j = 0; j < nrPerAxis; ++j)
		{
			WaterTriData vertex1;
			vertex1.thisX = -width / 2 + i * xStep;
			vertex1.thisY = -height / 2 + j * yStep;

			vertex1.oneX = -width / 2 + i * xStep + xStep;
			vertex1.oneY = -height / 2 + j * yStep;

			vertex1.twoX = -width / 2 + i * xStep + xStep;
			vertex1.twoY = -height / 2 + j * yStep + yStep;

			WaterTriData vertex2;
			vertex2.twoX = -width / 2 + i * xStep;
			vertex2.twoY = -height / 2 + j * yStep;

			vertex2.thisX = -width / 2 + i * xStep + xStep;
			vertex2.thisY = -height / 2 + j * yStep;

			vertex2.oneX = -width / 2 + i * xStep + xStep;
			vertex2.oneY = -height / 2 + j * yStep + yStep;

			WaterTriData vertex3;
			vertex3.oneX = -width / 2 + i * xStep;
			vertex3.oneY = -height / 2 + j * yStep;

			vertex3.twoX = -width / 2 + i * xStep + xStep;
			vertex3.twoY = -height / 2 + j * yStep;

			vertex3.thisX = -width / 2 + i * xStep + xStep;
			vertex3.thisY = -height / 2 + j * yStep + yStep;

			WaterTriData vertex21;
			vertex21.thisX = -width / 2 + i * xStep;
			vertex21.thisY = -height / 2 + j * yStep;

			vertex21.oneX = -width / 2 + i * xStep + xStep;
			vertex21.oneY = -height / 2 + j * yStep + yStep;

			vertex21.twoX = -width / 2 + i * xStep;
			vertex21.twoY = -height / 2 + j * yStep + yStep;

			WaterTriData vertex22;
			vertex22.twoX = -width / 2 + i * xStep;
			vertex22.twoY = -height / 2 + j * yStep;

			vertex22.thisX = -width / 2 + i * xStep + xStep;
			vertex22.thisY = -height / 2 + j * yStep + yStep;

			vertex22.oneX = -width / 2 + i * xStep;
			vertex22.oneY = -height / 2 + j * yStep + yStep;

			WaterTriData vertex23;
			vertex23.oneX = -width / 2 + i * xStep;
			vertex23.oneY = -height / 2 + j * yStep;

			vertex23.twoX = -width / 2 + i * xStep + xStep;
			vertex23.twoY = -height / 2 + j * yStep + yStep;

			vertex23.thisX = -width / 2 + i * xStep;
			vertex23.thisY = -height / 2 + j * yStep + yStep;

			vertices.push_back(vertex1);
			vertices.push_back(vertex2);
			vertices.push_back(vertex3);
			vertices.push_back(vertex21);
			vertices.push_back(vertex22);
			vertices.push_back(vertex23);
		}
	return vertices;
}

class FrameBuffer
{
public:
	FrameBuffer(int x, int y, int width, int height, bool depthstencil);
	void use() const;
	void reset(int width, int height) const;
	unsigned int getTexture() const { return texture; }
private:
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
	unsigned int fbo;
	unsigned int texture;
};

FrameBuffer::FrameBuffer(int x, int y, int width, int height, bool depthStencil)
{
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

	if (depthStencil)
	{
		unsigned int rbo;
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::exception("Framebuffer incomplete!");
	}
	
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void FrameBuffer::use() const
{
	glViewport(x, y, width, height);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void FrameBuffer::reset(int width, int height) const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
}

#include "stb_image.h"

float SkyBox::vertices[] = { -1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};

void SkyBox::draw(Window& window, Shader& shader)
{
	shader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	glBindVertexArray(VAO);
	glUniformMatrix4fv(glGetUniformLocation(shader.getID(), "projection"), 1, GL_FALSE, glm::value_ptr(window.getProjection()));
	glUniformMatrix4fv(glGetUniformLocation(shader.getID(), "view"), 1, GL_FALSE, glm::value_ptr(window.getView()));

	glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / sizeof(float));
}

SkyBox::SkyBox(const std::vector<std::string>& faces)
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	for (int i = 0; i < faces.size(); ++i)
	{
		int width, height, channels;
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &channels, 0);

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

int main()
{
	Window window(800, 600, "OPENGL");
	Shader shader(vertexShaderS, fragmentShaderS);
	Shader shader2(outlineShaderSVert, outlineShader);
	Shader spriteShaderProg(spriteShader, spriteFragShader);
	Shader postProcess(spriteShader, GaussianBlurShader);
	Shader waterShader(waterShaderVert, waterShaderFrag);
	Shader skyboxShader(cubemapVertS, cubemapFragS);
	SkyBox skay(std::vector<std::string>{"right.png", "left.png", "top.png", "bottom.png", "front.png", "back.png"});
	Model model("backpack.obj");
	Sprite sprite("window.png");
	WaterBody water(2, 2, 30, skay);
	water.setPosition(glm::vec3(0, -0.5f, 0));
	water.setRotation(glm::angleAxis(glm::radians(90.0f), glm::vec3(1, 0, 0)) * water.getRotation());
	FrameBuffer frameBuffer(0, 0, 800, 600, true);
	Sprite renderedToScreen(frameBuffer.getTexture());
	model.setScale(glm::vec3(0.2f, 0.2f, 0.2f));
	model.setPosition(glm::vec3(0, 0, -1));
	water.setScale(glm::vec3(15, 15, 1));
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float elapsedTime = 0;
	float time = glfwGetTime();
	while (!window.shouldClose())
	{
		float currentFrame = glfwGetTime();
		elapsedTime = currentFrame - time;
		window.processEvents(elapsedTime);
		frameBuffer.use();
		window.enableFaceCulling();
		window.clear();
		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		window.setProjection(glm::perspective(45.0f, (float)800 / 600, 0.1f, 100.0f));
		window.draw(model, shader);
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		window.draw(model, shader2);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);

		window.disableFaceCulling();
		//window.draw(sprite, spriteShaderProg);

		window.draw(water, waterShader);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);
		window.draw(skay, skyboxShader);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		frameBuffer.reset(800, 600);

		window.setProjection(glm::ortho(-0.5f,0.5f,-0.5f,0.5f,0.01f,100.0f));
		glm::mat4 oldView = window.getView();
		window.setView(glm::lookAt(glm::vec3(0, 0, 6), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));
		window.draw(renderedToScreen, postProcess);
		window.setView(oldView);
		window.swapBuffers();
		time = currentFrame;
	}
}