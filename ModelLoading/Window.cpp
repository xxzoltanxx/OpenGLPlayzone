#include "Window.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

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

Window::Window(unsigned int width, unsigned int height, const std::string& title, bool isFullscreen)
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
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_STENCIL_TEST);
	glViewport(0, 0, width, height);
	projection = glm::perspective(45.0f, (float)width / height, 0.1f, 100.0f);
	view = glm::lookAt(glm::vec3(0, 0, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
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
