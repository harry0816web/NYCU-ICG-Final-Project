#include "header/static_model.h"
#include "header/stb_image.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>

StaticModel::StaticModel(const std::string& path) {
    loadModel(path);
}

void StaticModel::loadModel(const std::string& path) {
    // Extract directory from path
    size_t lastSlash = path.find_last_of("/\\");
    directory = (lastSlash == std::string::npos) ? "" : path.substr(0, lastSlash + 1);
    
    // Check if file exists
    std::ifstream fileCheck(path);
    if (!fileCheck.good()) {
        std::cout << "ERROR::STATIC_MODEL:: File not found: " << path << std::endl;
        return;
    }
    fileCheck.close();
    
    // Import flags for OBJ files
    unsigned int importFlags = aiProcess_Triangulate 
                             | aiProcess_GenSmoothNormals 
                             | aiProcess_FlipUVs 
                             | aiProcess_CalcTangentSpace;
    
    std::cout << "Loading OBJ file: " << path << std::endl;
    m_scene = m_Importer.ReadFile(path, importFlags);
    
    if (!m_scene || m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_scene->mRootNode) {
        std::string errorString = m_Importer.GetErrorString();
        if (errorString.empty()) {
            errorString = "Unknown error";
        }
        std::cout << "ERROR::STATIC_MODEL:: " << errorString << std::endl;
        return;
    }
    
    std::cout << "OBJ file loaded successfully!" << std::endl;
    std::cout << "  - Meshes: " << m_scene->mNumMeshes << std::endl;
    std::cout << "  - Materials: " << m_scene->mNumMaterials << std::endl;
    
    // Load materials
    materials.resize(m_scene->mNumMaterials);
    for (unsigned int i = 0; i < m_scene->mNumMaterials; i++) {
        aiMaterial* mat = m_scene->mMaterials[i];
        Material& material = materials[i];
        
        // Initialize defaults
        material.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
        material.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
        material.specular = glm::vec3(1.0f, 1.0f, 1.0f);
        material.shininess = 32.0f;
        material.hasTexture = false;
        material.texture = 0;
        
        // Load ambient color
        aiColor3D ambient(0.0f, 0.0f, 0.0f);
        if (mat->Get(AI_MATKEY_COLOR_AMBIENT, ambient) == AI_SUCCESS) {
            material.ambient = glm::vec3(ambient.r, ambient.g, ambient.b);
        }
        
        // Load diffuse color
        aiColor3D diffuse(0.0f, 0.0f, 0.0f);
        if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS) {
            material.diffuse = glm::vec3(diffuse.r, diffuse.g, diffuse.b);
        }
        
        // Load specular color
        aiColor3D specular(0.0f, 0.0f, 0.0f);
        if (mat->Get(AI_MATKEY_COLOR_SPECULAR, specular) == AI_SUCCESS) {
            material.specular = glm::vec3(specular.r, specular.g, specular.b);
        }
        
        // Load shininess
        float shininess = 0.0f;
        if (mat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
            material.shininess = shininess;
        }
        
        // Load diffuse texture
        if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString str;
            mat->GetTexture(aiTextureType_DIFFUSE, 0, &str);
            std::string texturePath = directory + str.C_Str();
            material.texture = loadTextureFromFile(texturePath);
            material.hasTexture = (material.texture != 0);
        }
        
        std::cout << "  Material " << i << ": ambient(" << material.ambient.r << "," << material.ambient.g << "," << material.ambient.b << "), "
                  << "diffuse(" << material.diffuse.r << "," << material.diffuse.g << "," << material.diffuse.b << ")" << std::endl;
    }
    
    processNode(m_scene->mRootNode, m_scene);
    setupMesh();
    
    std::cout << "Static model processed: " << vertices.size() << " vertices, " << indices.size() << " indices" << std::endl;
}

void StaticModel::processNode(aiNode* node, const aiScene* scene) {
    // Process each mesh located at the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene);
    }
    
    // Recursively process each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

void StaticModel::processMesh(aiMesh* mesh, const aiScene* scene) {
    unsigned int vertexStart = vertices.size();
    
    // Process vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        StaticVertex vertex;
        
        // Position
        vertex.Position.x = mesh->mVertices[i].x;
        vertex.Position.y = mesh->mVertices[i].y;
        vertex.Position.z = mesh->mVertices[i].z;
        
        // Normals
        if (mesh->HasNormals()) {
            vertex.Normal.x = mesh->mNormals[i].x;
            vertex.Normal.y = mesh->mNormals[i].y;
            vertex.Normal.z = mesh->mNormals[i].z;
        } else {
            vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }
        
        // Texture coordinates
        if (mesh->mTextureCoords[0]) {
            vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
            vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;
        } else {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }
        
        vertices.push_back(vertex);
    }
    
    // Process indices and material indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(vertexStart + face.mIndices[j]);
        }
        
        // Store material index for this face
        unsigned int materialIndex = mesh->mMaterialIndex;
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            materialIndices.push_back(materialIndex);
        }
    }
}

unsigned int StaticModel::loadTextureFromFile(const std::string& path) {
    // Check cache first
    if (textureCache.find(path) != textureCache.end()) {
        return textureCache[path];
    }
    
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(false);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    
    if (data) {
        GLenum format;
        if (nrChannels == 1) format = GL_RED;
        else if (nrChannels == 3) format = GL_RGB;
        else if (nrChannels == 4) format = GL_RGBA;
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        stbi_image_free(data);
        textureCache[path] = textureID;
    } else {
        std::cout << "Failed to load texture: " << path << std::endl;
        stbi_image_free(data);
        textureID = 0;
    }
    
    return textureID;
}

void StaticModel::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(StaticVertex), &vertices[0], GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    
    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(StaticVertex), (void*)0);
    
    // Vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(StaticVertex), (void*)offsetof(StaticVertex, Normal));
    
    // Vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(StaticVertex), (void*)offsetof(StaticVertex, TexCoords));
    
    glBindVertexArray(0);
}

unsigned int StaticModel::createColorTexture(const glm::vec3& color) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    unsigned char data[3] = {
        (unsigned char)(glm::clamp(color.r, 0.0f, 1.0f) * 255),
        (unsigned char)(glm::clamp(color.g, 0.0f, 1.0f) * 255),
        (unsigned char)(glm::clamp(color.b, 0.0f, 1.0f) * 255)
    };
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    return textureID;
}

unsigned int StaticModel::getMaterialTexture(unsigned int materialIndex) {
    if (materialIndex >= materials.size()) {
        materialIndex = 0; // Use first material as fallback
    }
    
    // Check cache first
    if (materialTextureCache.find(materialIndex) != materialTextureCache.end()) {
        return materialTextureCache[materialIndex];
    }
    
    Material& mat = materials[materialIndex];
    
    // If material has texture, use it
    if (mat.hasTexture && mat.texture != 0) {
        materialTextureCache[materialIndex] = mat.texture;
        return mat.texture;
    }
    
    // Otherwise, create color texture from diffuse color
    unsigned int colorTexture = createColorTexture(mat.diffuse);
    materialTextureCache[materialIndex] = colorTexture;
    return colorTexture;
}

void StaticModel::render() {
    glBindVertexArray(VAO);
    
    // Group faces by material and render each group
    if (materialIndices.size() == 0 || materialIndices.size() != indices.size()) {
        // Fallback: render all at once with first material
        if (materials.size() > 0) {
            unsigned int tex = getMaterialTexture(0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex);
        }
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    } else {
        // Render by material groups
        // materialIndices stores material index for each index (3 indices per triangle)
        // So we need to group triangles by material
        
        unsigned int numTriangles = indices.size() / 3;
        unsigned int currentMaterial = (numTriangles > 0 && materialIndices.size() > 0) ? materialIndices[0] : 0;
        unsigned int startTriangle = 0;
        
        for (unsigned int tri = 0; tri <= numTriangles; tri++) {
            unsigned int matIndex = (tri < numTriangles && tri * 3 < materialIndices.size()) ? materialIndices[tri * 3] : currentMaterial;
            
            // Check if we need to switch material or reached the end
            if (tri == numTriangles || matIndex != currentMaterial) {
                // Render current group
                if (startTriangle < tri) {
                    unsigned int tex = getMaterialTexture(currentMaterial);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, tex);
                    
                    unsigned int startIdx = startTriangle * 3;
                    unsigned int count = (tri - startTriangle) * 3;
                    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (void*)(startIdx * sizeof(unsigned int)));
                }
                
                if (tri < numTriangles) {
                    // Switch to next material
                    currentMaterial = matIndex;
                    startTriangle = tri;
                }
            }
        }
    }
    
    glBindVertexArray(0);
}

