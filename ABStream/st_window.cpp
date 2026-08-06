#include "st_window.h"

#include "st_settings_controller.h"


namespace st {

	StWindow::StWindow( std::string name) :
	width{ StSettingsManager::getManager().cubemapResolution },
	height{ StSettingsManager::getManager().cubemapResolution },
	headless{ StSettingsManager::getManager().headless },
	windowName{ name } {
		if (!headless)
			initWindow();
	}
	StWindow::~StWindow() {
		if (StSettingsManager::getManager().headless)
			return;
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void StWindow::initWindow() {
		
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE,GLFW_FALSE);
		window = glfwCreateWindow(width,height,windowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window,this);
		glfwSetFramebufferSizeCallback(window,framebufferResizeCallback);

	}

	bool StWindow::shouldClose() const
	{
		if (StSettingsManager::getManager().headless)
			return false;
		return glfwWindowShouldClose(window);
	}

	void StWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		VkResult res = glfwCreateWindowSurface(instance, window, nullptr, surface);
		if ( res != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface");
		}
	}

	void StWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto stWindow = reinterpret_cast<StWindow*>(glfwGetWindowUserPointer(window));
		stWindow->width = width;
		stWindow->height = height;
		stWindow->framebufferResized = true;
	}

}
