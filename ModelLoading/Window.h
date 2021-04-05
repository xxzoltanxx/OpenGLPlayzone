#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "Drawable.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <string>

class Window
{
public:
	Window(unsigned int width, unsigned int height, const std::string& title, bool fpsCamera = true, bool isFullscreen = false);
	~Window();
	bool shouldClose() const;
	void clear() const;
	void swapBuffers() const;
	void draw(Drawable& drawable, Shader& shader) { drawable.draw(*this, shader); }
	void setClearColor(const glm::vec4& clearColor) { this->clearColor = clearColor; }

	void setCameraPosition(const glm::vec3& pos) { cameraPosition = pos; view = glm::toMat4(cameraRotation) * glm::translate(glm::mat4(1.0f), cameraPosition); }
	void rotateCamera(const glm::fquat& rot) { cameraRotation = rot * cameraRotation; view = glm::toMat4(cameraRotation) * glm::translate(glm::mat4(1.0f), cameraPosition); }
	void setView(const glm::vec3& pos, const glm::vec3& center, const glm::vec3& up) { this->view = glm::lookAt(pos, center, up); this->cameraPosition = pos; }
	void setView(const glm::mat4& view) { this->view = view; }
	void enableFaceCulling() const;
	void disableFaceCulling() const;
	void setProjection(const glm::mat4& proj) { this->projection = proj; }

	void processEvents(float dt);

	glm::vec3 getCameraPosition() const { return cameraPosition; }
	const glm::mat4& getView() const { return view; }
	const glm::mat4& getProjection() const { return projection; }


	glm::vec3 front = glm::vec3(0, 0, 1);
	glm::vec3 side = glm::vec3(1, 0 , 0);
	float pitch = 0;
	float yaw = 0;

	double oldX = 0;
	double oldY = 0;


	glm::fquat cameraRotation = glm::fquat(1, 0, 0, 0);
private:
	GLFWwindow* window;
	static bool glfwInited;
	static int numWindows;
	glm::vec4 clearColor = glm::vec4(0, 0, 0, 1);
	glm::mat4 projection;
	glm::mat4 view;
	glm::vec3 cameraPosition = glm::vec3(0, 0, -3);
};