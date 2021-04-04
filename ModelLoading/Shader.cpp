#include "Shader.h"
#include <glad/glad.h>
#include <iostream>

void Shader::checkShader(int id, bool link)
{
	int sucess;
	glGetShaderiv(id, link ? GL_LINK_STATUS : GL_COMPILE_STATUS, &sucess);
	if (!sucess)
	{
		char infolog[512];
		glGetShaderInfoLog(id, 512, nullptr, infolog);
		std::cout << infolog;
	}
}

void Shader::use()
{
	glUseProgram(ID);
}

Shader::Shader(const char* vertSource, const char* fragSource)
{
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vertexShader, 1, &vertSource, nullptr);
	glShaderSource(fragmentShader, 1, &fragSource, nullptr);

	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);

	checkShader(vertexShader);
	checkShader(fragmentShader);

	ID = glCreateProgram();
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);

	checkShader(ID, true);

	glLinkProgram(ID);
}