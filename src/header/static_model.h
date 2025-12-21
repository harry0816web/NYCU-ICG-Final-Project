#ifndef STATIC_MODEL_H
#define STATIC_MODEL_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <string>
#include <map>

struct StaticVertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
    bool hasTexture;
    unsigned int texture;
};

class StaticModel {
public:
    std::vector<StaticVertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Material> materials;
    std::vector<unsigned int> materialIndices; // Which material each face uses
    
    unsigned int VAO, VBO, EBO;
    
    const aiScene* m_scene;
    Assimp::Importer m_Importer;
    
    StaticModel(const std::string& path);
    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    void processMesh(aiMesh* mesh, const aiScene* scene);
    void loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& directory);
    void setupMesh();
    void render();
    unsigned int getMaterialTexture(unsigned int materialIndex);
    
private:
    std::string directory;
    std::map<std::string, unsigned int> textureCache;
    std::map<unsigned int, unsigned int> materialTextureCache; // Cache for material color textures
    unsigned int loadTextureFromFile(const std::string& path);
    unsigned int createColorTexture(const glm::vec3& color);
};

#endif

