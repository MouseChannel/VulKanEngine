#include "../base.h"

namespace VK::Wrapper {

class Window {

private:
  GLFWwindow *m_Window{NULL};
  int m_Width{0};
  int m_Height{0};

public:
  using Ptr = std::shared_ptr<Window>;
  static Ptr Create(const int &width, const int &height) {
    return std::make_shared<Window>(width, height);
  }
  Window(const int &width, const int &height);
  ~Window();
  bool ShouldClose();
  void PollEvent();
  [[nodiscard]] auto GetWindow() const { return m_Window; }
  bool m_WindowResized{false};
  static void WindowResized(GLFWwindow *window, int width, int height);
};

Window::Window(const int &width, const int &height) {

  m_Width = width;
  m_Height = height;
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  m_Window = glfwCreateWindow(m_Width, m_Height, "mochengWindow", nullptr, nullptr);
  if (!m_Window) {
    std::cerr << "Error: failed to create window" << std::endl;
  }
  glfwSetWindowUserPointer(m_Window, this);
  glfwSetFramebufferSizeCallback(m_Window, WindowResized);
 
}
Window::~Window() {
  glfwDestroyWindow(m_Window);
  glfwTerminate();
}

bool Window::ShouldClose() { return glfwWindowShouldClose(m_Window); }

void Window::PollEvent() { glfwPollEvents(); }
void Window::WindowResized(GLFWwindow *window, int width, int height) {
  auto pUserData = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  pUserData->m_WindowResized = true;
}
} // namespace VK::Wrapper