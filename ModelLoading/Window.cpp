#include "Window.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <functional>

bool Window::glfwInited = false;

int Window::numWindows = 0;

Window::~Window()
{
	--numWindows;
	if (numWindows == 0)
	{
		glfwTerminate();
	}
}

void Window::processEvents(float dt)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		cameraPosition += front * dt;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		cameraPosition -= front * dt;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		cameraPosition += -side * dt;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		cameraPosition += side * dt;
	}
	setCameraPosition(cameraPosition);
}
void mousePosCallback(GLFWwindow* window, double x, double y)
{
	Window* user = (Window*)glfwGetWindowUserPointer(window);
	if (user->oldX == 0)
	{
		user->oldX = x;
		user->oldY = y;
	}
	float deltaX = (user->oldY - y) * 0.1f;
	float deltaY = (user->oldX - x) * 0.1f;


	user->oldX = x;
	user->oldY = y;
	user->pitch -= deltaX;
	user->yaw -= deltaY;

	if (user->pitch < -90)
	{
		user->pitch = -90;
	}
	if (user->pitch > 90)
	{
		user->pitch = 90;
	}
	auto pitch = glm::angleAxis(glm::radians(user->pitch), glm::vec3(1, 0, 0));
	auto yaw = glm::angleAxis(glm::radians(user->yaw), glm::vec3(0, 1, 0));
	auto yaw2 = glm::angleAxis(-glm::radians(user->yaw), glm::vec3(0, 1, 0));
	auto pitch2 = glm::angleAxis(-glm::radians(user->pitch), glm::vec3(1, 0, 0));
	user->front = yaw2 * pitch2 * glm::vec3(0, 0, -1);
	user->front = -glm::normalize(user->front);
	user->side = yaw2 * pitch2 * glm::vec3(1, 0, 0);
	user->side = -glm::normalize(user->side);

	user->cameraRotation = pitch * yaw;
	user->setView(glm::toMat4(pitch * yaw) * glm::translate(glm::mat4(1.0f), user->getCameraPosition()));
}

Window::Window(unsigned int width, unsigned int height, const std::string& title, bool fpsCamera, bool isFullscreen)
{
	if (!glfwInited)
	{
		glfwInited = true;
		glfwInit();
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(width, height, title.c_str(), isFullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSetWindowUserPointer(window, this);
	if (fpsCamera)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPosCallback(window, mousePosCallback);
	}
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_STENCIL_TEST);
	glViewport(0, 0, width, height);
	projection = glm::perspective(45.0f, (float)width / height, 0.1f, 100.0f);
	view = glm::toMat4(cameraRotation) * glm::translate(glm::mat4(1.0f), cameraPosition);
	++numWindows;
}

void Window::enableFaceCulling() const
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}

void Window::disableFaceCulling() const
{
	glDisable(GL_CULL_FACE);
}

void Window::swapBuffers() const
{
	glfwSwapBuffers(window);
}

void Window::clear() const
{
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glfwPollEvents();
}

bool Window::shouldClose() const
{
	return glfwWindowShouldClose(window);
}
