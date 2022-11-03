cmake_minimum_required(VERSION 3.0.0)
project(vk VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/lib/Include  ${PROJECT_SOURCE_DIR}/lib )

FIND_LIBRARY(VULKAN vulkan ${PROJECT_SOURCE_DIR}/lib)
FIND_LIBRARY(GLFW libglfw3 ${PROJECT_SOURCE_DIR}/lib)

# link_libraries(${VULKAN}  ${GLFW3})
# aux_source_directory(. DIRSRCS)
# add_subdirectory(VulkanWrapper)
add_executable(vk main.cpp)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
target_link_libraries(vk ${VULKAN} ${GLFW})
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -I${PROJECT_SOURCE_DIR}/lib")
# "D:\\SomeSoft\\LLVM_MINGW\\include\\**"
