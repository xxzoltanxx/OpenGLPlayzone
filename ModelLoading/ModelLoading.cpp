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
	}
)";


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

int main()
{
	Window window(800, 600, "OPENGL", false);
	Shader shader(vertexShaderS, fragmentShaderS);
	Shader shader2(outlineShaderSVert, outlineShader);
	Shader spriteShaderProg(spriteShader, spriteFragShader);
	Shader postProcess(spriteShader, GaussianBlurShader);
	Model model("backpack.obj");
	Sprite sprite("window.png");
	FrameBuffer frameBuffer(0, 0, 800, 600, true);
	Sprite renderedToScreen(frameBuffer.getTexture());
	model.setScale(glm::vec3(0.2f, 0.2f, 0.2f));
	model.setPosition(glm::vec3(0, 0, -1));
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	while (!window.shouldClose())
	{
		frameBuffer.use();
		window.enableFaceCulling();
		window.clear();
		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		window.setProjection(glm::perspective(45.0f, (float)800 / 600, 0.1f, 100.0f));
		window.setView(glm::lookAt(glm::vec3(2 * sin(glfwGetTime() * 0.2f), 0, 2 * cos(glfwGetTime() * 0.2f)), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));
		window.draw(model, shader);
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		window.draw(model, shader2);

		window.disableFaceCulling();
		window.draw(sprite, spriteShaderProg);
		frameBuffer.reset(800, 600);

		window.setProjection(glm::ortho(-0.5f,0.5f,-0.5f,0.5f,0.01f,100.0f));
		window.setView(glm::lookAt(glm::vec3(0, 0, 6), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));
		window.draw(renderedToScreen, postProcess);

		window.swapBuffers();
	}
}