#pragma once

// #include ""

#include <array>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>


#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLFW_INCLUDE_VULKAN
#include <header/glfw3.h>
#include <vulkan/vulkan.h>
const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};
struct VPMatrices {
  glm::mat4 mViewMatrix;
  glm::mat4 mProjectionMatrix;

  VPMatrices() {
    mViewMatrix = glm::mat4(1.0f);
    mProjectionMatrix = glm::mat4(1.0f);
  }
};

struct ObjectUniform {
  glm::mat4 mModelMatrix;

  ObjectUniform() { mModelMatrix = glm::mat4(1.0f); }
};