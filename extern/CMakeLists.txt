add_subdirectory("glfw")
add_subdirectory("glm")
add_subdirectory("glad")

# Force assimp to use its built-in minizip to avoid architecture mismatch issues
set(ASSIMP_BUILD_MINIZIP ON CACHE BOOL "Build minizip from sources" FORCE)
# Prevent assimp from finding system minizip
set(UNZIP_FOUND OFF CACHE BOOL "Force use of built-in unzip" FORCE)
add_subdirectory("assimp")