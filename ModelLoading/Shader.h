#pragma once

class Shader
{
public:
	Shader(const char* vertSource, const char* fragSource);
	unsigned int getID() const { return ID; }
	void use();
private:
	void checkShader(int id, bool link = false);
	unsigned int ID;
};