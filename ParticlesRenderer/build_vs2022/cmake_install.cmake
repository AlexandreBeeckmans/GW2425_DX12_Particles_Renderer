# Install script for directory: C:/DAE/2024-2025/1st_semester/GradWork/CodeSamples/DirectX12/GW2425_DX12_Particles_Renderer/ParticlesRenderer

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/LearningDirectX12")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/DAE/2024-2025/1st_semester/GradWork/CodeSamples/DirectX12/GW2425_DX12_Particles_Renderer/ParticlesRenderer/build_vs2022/extern/assimp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/DAE/2024-2025/1st_semester/GradWork/CodeSamples/DirectX12/GW2425_DX12_Particles_Renderer/ParticlesRenderer/build_vs2022/extern/gainput/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/DAE/2024-2025/1st_semester/GradWork/CodeSamples/DirectX12/GW2425_DX12_Particles_Renderer/ParticlesRenderer/build_vs2022/extern/DirectXTex/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/DAE/2024-2025/1st_semester/GradWork/CodeSamples/DirectX12/GW2425_DX12_Particles_Renderer/ParticlesRenderer/build_vs2022/GameFramework/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/DAE/2024-2025/1st_semester/GradWork/CodeSamples/DirectX12/GW2425_DX12_Particles_Renderer/ParticlesRenderer/build_vs2022/DX12Lib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/DAE/2024-2025/1st_semester/GradWork/CodeSamples/DirectX12/GW2425_DX12_Particles_Renderer/ParticlesRenderer/build_vs2022/samples/03-Textures/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "C:/DAE/2024-2025/1st_semester/GradWork/CodeSamples/DirectX12/GW2425_DX12_Particles_Renderer/ParticlesRenderer/build_vs2022/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")