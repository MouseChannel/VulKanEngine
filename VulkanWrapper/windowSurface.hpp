#pragma once

#include "../base.h"
#include "Instance.hpp"
#include "window.hpp"
namespace VK::Wrapper {

class WindowSurface {

private:
  VkSurfaceKHR m_Surface{VK_NULL_HANDLE};
  Instance::Ptr m_Instance{nullptr};

public:
  using Ptr = std::shared_ptr<WindowSurface>;
  static Ptr Create(Instance::Ptr instance, Window::Ptr window) {
    return std::make_shared<WindowSurface>(instance, window);
  }
  WindowSurface(Instance::Ptr instance, Window::Ptr window);

  ~WindowSurface();
  [[nodiscard]] auto GetSurface() { return m_Surface; }
};

WindowSurface::WindowSurface(Instance::Ptr instance, Window::Ptr window) {
  m_Instance = instance;
 
   if (glfwCreateWindowSurface(instance->GetInstance(), window->GetWindow(),
                              nullptr, &m_Surface) != VK_SUCCESS) {
    throw std::runtime_error("Error: failed to create surface");
  }
}

WindowSurface::~WindowSurface() {
  vkDestroySurfaceKHR(m_Instance->GetInstance(), m_Surface, nullptr);
  m_Instance.reset();
}
} // namespace VK::Wrapper