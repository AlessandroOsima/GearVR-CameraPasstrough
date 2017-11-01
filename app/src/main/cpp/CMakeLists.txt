#
# Copyright (C)  2017 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_VERBOSE_MAKEFILE on)
set(COMMON_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../../../../common)
set(VR_API_LOCATION C:/AnotheReality/Projects/Android/ovr_sdk_mobile_1.7.0/VrApi)
set(BUILD_CONFIG Release)

# now build app's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Werror")

# Export ANativeActivity_onCreate(),
# Refer to: https://github.com/android-ndk/ndk/issues/381.
set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Logger)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Shader)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Math)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Mesh)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Texture)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Framebuffer)

add_subdirectory(Logger)
add_subdirectory(Shader)
add_subdirectory(glm)
add_subdirectory(Math)
add_subdirectory(Mesh)
add_subdirectory(Texture)
add_subdirectory(Framebuffer)

add_library(camera_textureview SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GLUtilities.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Logger/Assert.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Logger/Logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Shader/ShaderProgram.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Math/Filters.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Math/MathTypes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Mesh/VertexData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Mesh/Renderable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Texture/Texture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Texture/Sampler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Framebuffer/Framebuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/native-lib.cpp)

# add include path
target_include_directories(camera_textureview PRIVATE ${COMMON_SOURCE_DIR})

add_library( vrapi SHARED IMPORTED )

set_target_properties( # Specifies the target library.
             vrapi

             # Specifies the parameter you want to define.
             PROPERTIES IMPORTED_LOCATION

             # Provides the path to the library you want to import.
              ${CMAKE_CURRENT_SOURCE_DIR}/../lib/${ANDROID_ABI}/libvrapi.so )

add_definitions(-D_DEBUG)

include_directories(${VR_API_LOCATION}/Include)

# add lib dependencies
target_link_libraries(camera_textureview dl android log m camera2ndk EGL GLESv3 vrapi)
