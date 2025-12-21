# Install script for directory: /Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
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

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "libassimp6.0.2-dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/lib/libassimp.a")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libassimp.a" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libassimp.a")
    execute_process(COMMAND "/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libassimp.a")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "assimp-dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/assimp" TYPE FILE FILES
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/anim.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/aabb.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/ai_assert.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/camera.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/color4.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/color4.inl"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/config.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/ColladaMetaData.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/commonMetaData.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/defs.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/cfileio.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/light.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/material.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/material.inl"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/matrix3x3.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/matrix3x3.inl"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/matrix4x4.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/matrix4x4.inl"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/mesh.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/ObjMaterial.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/pbrmaterial.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/GltfMaterial.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/postprocess.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/quaternion.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/quaternion.inl"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/scene.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/metadata.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/texture.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/types.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/vector2.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/vector2.inl"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/vector3.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/vector3.inl"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/version.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/cimport.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/AssertHandler.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/importerdesc.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/Importer.hpp"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/DefaultLogger.hpp"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/ProgressHandler.hpp"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/IOStream.hpp"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/IOSystem.hpp"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/Logger.hpp"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/LogStream.hpp"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/NullLogger.hpp"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/cexport.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/Exporter.hpp"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/DefaultIOStream.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/DefaultIOSystem.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/ZipArchiveIOSystem.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/SceneCombiner.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/fast_atof.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/qnan.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/BaseImporter.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/Hash.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/MemoryIOWrapper.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/ParsingUtils.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/StreamReader.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/StreamWriter.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/StringComparison.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/StringUtils.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/SGSpatialSort.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/GenericProperty.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/SpatialSort.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/SkeletonMeshBuilder.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/SmallVector.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/SmoothingGroups.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/SmoothingGroups.inl"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/StandardShapes.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/RemoveComments.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/Subdivision.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/Vertex.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/LineSplitter.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/TinyFormatter.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/Profiler.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/LogAux.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/Bitmap.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/XMLTools.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/IOStreamBuffer.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/CreateAnimMesh.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/XmlParser.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/BlobIOSystem.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/MathFunctions.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/Exceptional.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/ByteSwapper.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/Base64.hpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "assimp-dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/assimp/Compiler" TYPE FILE FILES
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/Compiler/pushpack1.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/Compiler/poppack1.h"
    "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/../include/assimp/Compiler/pstdint.h"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/Users/harryp/Desktop/Courses/ICG/ICG_2025_HW3_animation/extern/assimp/code/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
