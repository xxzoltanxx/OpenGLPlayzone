#pragma once

class Window;
class Shader;

class Drawable
{
public:
	virtual void draw(Window& window, Shader& shader) = 0;
};
