#include "header/animated_model.h"
#include "header/stb_image.h"
#include <iostream>
#include <fstream>

AnimatedModel::AnimatedModel(const std::string& path) {
    loadModel(path);
}

void AnimatedModel::loadModel(const std::string& path) {
    // Check if file exists
    std::ifstream fileCheck(path);
    if (!fileCheck.good()) {
        std::cout << "ERROR::ASSIMP:: File not found: " << path << std::endl;
        return;
    }
    fileCheck.close();
    
    // Import flags optimized for Mixamo FBX files
    unsigned int importFlags = aiProcess_Triangulate 
                             | aiProcess_GenSmoothNormals 
                             | aiProcess_FlipUVs 
                             | aiProcess_CalcTangentSpace
                             | aiProcess_LimitBoneWeights;  // Limit bone weights for better performance
    
    // Configure FBX importer settings for Mixamo files
    m_Importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_ANIMATIONS, true);
    m_Importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_WEIGHTS, true);
    m_Importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);  // Mixamo doesn't need this
    m_Importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_STRICT_MODE, false);  // Allow different FBX versions
    
    std::cout << "Loading FBX file: " << path << std::endl;
    m_scene = m_Importer.ReadFile(path, importFlags);
    
    if (!m_scene) {
        std::string errorString = m_Importer.GetErrorString();
        if (errorString.empty()) {
            errorString = "Unknown error - file may be corrupted or unsupported format";
        }
        std::cout << "ERROR::ASSIMP:: Failed to load file: " << path << std::endl;
        std::cout << "ERROR::ASSIMP:: " << errorString << std::endl;
        return;
    }
    
    if (m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        std::cout << "WARNING::ASSIMP:: Scene is incomplete: " << m_Importer.GetErrorString() << std::endl;
    }
    
    if (!m_scene->mRootNode) {
        std::cout << "ERROR::ASSIMP:: Scene has no root node" << std::endl;
        return;
    }
    
    std::cout << "FBX file loaded successfully!" << std::endl;
    std::cout << "  - Meshes: " << m_scene->mNumMeshes << std::endl;
    std::cout << "  - Animations: " << m_scene->mNumAnimations << std::endl;
    std::cout << "  - Materials: " << m_scene->mNumMaterials << std::endl;
    
    // Initialize bone matrices (increase to 200 for safety with Mixamo)
    m_FinalBoneMatrices.resize(200, glm::mat4(1.0f)); // Reserve space for 200 bones
    
    // Set current animation if available
    if (m_scene->mNumAnimations > 0) {
        m_CurrentAnimation = m_scene->mAnimations[0];
        std::cout << "  - Animation name: " << m_CurrentAnimation->mName.C_Str() << std::endl;
        std::cout << "  - Animation duration: " << m_CurrentAnimation->mDuration << " ticks" << std::endl;
        std::cout << "  - Animation ticks per second: " << m_CurrentAnimation->mTicksPerSecond << std::endl;
        std::cout << "  - Animation channels: " << m_CurrentAnimation->mNumChannels << std::endl;
    } else {
        std::cout << "WARNING:: No animations found in FBX file!" << std::endl;
    }
    
    processNode(m_scene->mRootNode, m_scene);
    setupMesh();
    
    std::cout << "Model processed: " << vertices.size() << " vertices, " << indices.size() << " indices" << std::endl;
    std::cout << "Bones loaded: " << m_BoneCounter << std::endl;
}

void AnimatedModel::processNode(aiNode* node, const aiScene* scene) {
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

void AnimatedModel::processMesh(aiMesh* mesh, const aiScene* scene) {
    // Process vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        setVertexBoneDataToDefault(vertex);
        
        // Position
        vertex.Position.x = mesh->mVertices[i].x;
        vertex.Position.y = mesh->mVertices[i].y;
        vertex.Position.z = mesh->mVertices[i].z;
        
        // Normals
        if (mesh->HasNormals()) {
            vertex.Normal.x = mesh->mNormals[i].x;
            vertex.Normal.y = mesh->mNormals[i].y;
            vertex.Normal.z = mesh->mNormals[i].z;
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
    
    // Process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    
    // Process bone weights
    extractBoneWeightForVertices(vertices, mesh, scene);
}

void AnimatedModel::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    
    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    
    // Vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    
    // Vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    
    // Bone IDs
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));
    
    // Bone weights
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
    
    glBindVertexArray(0);
}

void AnimatedModel::loadTexture(const std::string& filepath) {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(false);
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format;
        if (nrChannels == 1) format = GL_RED;
        else if (nrChannels == 3) format = GL_RGB;
        else if (nrChannels == 4) format = GL_RGBA;
        
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture: " << filepath << std::endl;
    }
    stbi_image_free(data);
}

void AnimatedModel::render() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void AnimatedModel::updateAnimation(float timeInSeconds) {
    if (!m_CurrentAnimation) return;
    
    // Handle Mixamo FBX files where mTicksPerSecond might be 0
    double ticksPerSecond = m_CurrentAnimation->mTicksPerSecond;
    if (ticksPerSecond == 0.0) {
        ticksPerSecond = 25.0; // Default to 25 FPS for Mixamo animations
    }
    
    m_AnimationTime = fmod(timeInSeconds * ticksPerSecond, m_CurrentAnimation->mDuration);
    calculateBoneTransform(m_scene->mRootNode, glm::mat4(1.0f), m_AnimationTime);
}

void AnimatedModel::calculateBoneTransform(const aiNode* node, glm::mat4 parentTransform, float animationTime) {
    std::string nodeName = node->mName.data;
    glm::mat4 nodeTransform = aiMatrix4x4ToGlm(node->mTransformation);
    
    const aiNodeAnim* nodeAnim = nullptr;
    
    // Find the animation node for this bone
    if (m_CurrentAnimation) {
        for (unsigned int i = 0; i < m_CurrentAnimation->mNumChannels; i++) {
            if (std::string(m_CurrentAnimation->mChannels[i]->mNodeName.data) == nodeName) {
                nodeAnim = m_CurrentAnimation->mChannels[i];
                break;
            }
        }
    }
    
    if (nodeAnim) {
        // Interpolate scaling and generate scaling transformation matrix
        glm::vec3 scaling;
        if (nodeAnim->mNumScalingKeys == 0) {
            scaling = glm::vec3(1.0f);
        } else if (nodeAnim->mNumScalingKeys == 1) {
            scaling = aiVector3DToGlm(nodeAnim->mScalingKeys[0].mValue);
        } else {
            unsigned int scalingIndex = nodeAnim->mNumScalingKeys - 2; // Default to last pair
            for (unsigned int i = 0; i < nodeAnim->mNumScalingKeys - 1; i++) {
                if (animationTime < (float)nodeAnim->mScalingKeys[i + 1].mTime) {
                    scalingIndex = i;
                    break;
                }
            }
            
            float deltaTime = nodeAnim->mScalingKeys[scalingIndex + 1].mTime - nodeAnim->mScalingKeys[scalingIndex].mTime;
            float factor = 0.0f;
            if (deltaTime > 0.0001f) {
                factor = (animationTime - (float)nodeAnim->mScalingKeys[scalingIndex].mTime) / deltaTime;
                if (factor < 0.0f) factor = 0.0f;
                if (factor > 1.0f) factor = 1.0f;
            }
            
            glm::vec3 start = aiVector3DToGlm(nodeAnim->mScalingKeys[scalingIndex].mValue);
            glm::vec3 end = aiVector3DToGlm(nodeAnim->mScalingKeys[scalingIndex + 1].mValue);
            scaling = glm::mix(start, end, factor);
        }
        
        // Interpolate rotation and generate rotation transformation matrix
        glm::quat rotation;
        if (nodeAnim->mNumRotationKeys == 0) {
            rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
        } else if (nodeAnim->mNumRotationKeys == 1) {
            rotation = aiQuaternionToGlm(nodeAnim->mRotationKeys[0].mValue);
        } else {
            unsigned int rotationIndex = nodeAnim->mNumRotationKeys - 2; // Default to last pair
            for (unsigned int i = 0; i < nodeAnim->mNumRotationKeys - 1; i++) {
                if (animationTime < (float)nodeAnim->mRotationKeys[i + 1].mTime) {
                    rotationIndex = i;
                    break;
                }
            }
            
            float deltaTime = nodeAnim->mRotationKeys[rotationIndex + 1].mTime - nodeAnim->mRotationKeys[rotationIndex].mTime;
            float factor = 0.0f;
            if (deltaTime > 0.0001f) {
                factor = (animationTime - (float)nodeAnim->mRotationKeys[rotationIndex].mTime) / deltaTime;
                if (factor < 0.0f) factor = 0.0f;
                if (factor > 1.0f) factor = 1.0f;
            }
            
            glm::quat start = aiQuaternionToGlm(nodeAnim->mRotationKeys[rotationIndex].mValue);
            glm::quat end = aiQuaternionToGlm(nodeAnim->mRotationKeys[rotationIndex + 1].mValue);
            rotation = glm::slerp(start, end, factor);
        }
        
        // Interpolate translation and generate translation transformation matrix
        glm::vec3 translation;
        if (nodeAnim->mNumPositionKeys == 0) {
            translation = glm::vec3(0.0f);
        } else if (nodeAnim->mNumPositionKeys == 1) {
            translation = aiVector3DToGlm(nodeAnim->mPositionKeys[0].mValue);
        } else {
            unsigned int positionIndex = nodeAnim->mNumPositionKeys - 2; // Default to last pair
            for (unsigned int i = 0; i < nodeAnim->mNumPositionKeys - 1; i++) {
                if (animationTime < (float)nodeAnim->mPositionKeys[i + 1].mTime) {
                    positionIndex = i;
                    break;
                }
            }
            
            float deltaTime = nodeAnim->mPositionKeys[positionIndex + 1].mTime - nodeAnim->mPositionKeys[positionIndex].mTime;
            float factor = 0.0f;
            if (deltaTime > 0.0001f) {
                factor = (animationTime - (float)nodeAnim->mPositionKeys[positionIndex].mTime) / deltaTime;
                if (factor < 0.0f) factor = 0.0f;
                if (factor > 1.0f) factor = 1.0f;
            }
            
            glm::vec3 start = aiVector3DToGlm(nodeAnim->mPositionKeys[positionIndex].mValue);
            glm::vec3 end = aiVector3DToGlm(nodeAnim->mPositionKeys[positionIndex + 1].mValue);
            translation = glm::mix(start, end, factor);
        }
        
        // Combine transformations
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), translation);
        glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scaling);
        
        nodeTransform = translationMatrix * rotationMatrix * scaleMatrix;
    }
    
    // Apply additional rotation if specified (for cinematic control)
    auto additionalRotIt = m_AdditionalBoneRotations.find(nodeName);
    if (additionalRotIt != m_AdditionalBoneRotations.end()) {
        // Extract translation (last column)
        glm::vec3 translation = glm::vec3(nodeTransform[3]);
        
        // Extract scale (length of first 3 columns)
        glm::vec3 scale = glm::vec3(
            glm::length(glm::vec3(nodeTransform[0])),
            glm::length(glm::vec3(nodeTransform[1])),
            glm::length(glm::vec3(nodeTransform[2]))
        );
        
        // Extract rotation (normalize columns to remove scale)
        glm::mat3 rotMat = glm::mat3(
            glm::normalize(glm::vec3(nodeTransform[0])),
            glm::normalize(glm::vec3(nodeTransform[1])),
            glm::normalize(glm::vec3(nodeTransform[2]))
        );
        glm::quat currentRotation = glm::quat_cast(rotMat);
        
        // Combine rotations: additional rotation applied after animation rotation
        // This means: finalRotation = additionalRotation * animationRotation
        glm::quat combinedRotation = additionalRotIt->second * currentRotation;
        
        // Reconstruct transform with combined rotation
        glm::mat4 combinedRotMatrix = glm::mat4_cast(combinedRotation);
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
        glm::mat4 transMatrix = glm::translate(glm::mat4(1.0f), translation);
        
        nodeTransform = transMatrix * combinedRotMatrix * scaleMatrix;
    }
    
    glm::mat4 globalTransformation = parentTransform * nodeTransform;
    
    auto boneInfoMap = m_BoneInfoMap.find(nodeName);
    if (boneInfoMap != m_BoneInfoMap.end()) {
        int index = boneInfoMap->second.id;
        glm::mat4 offset = boneInfoMap->second.offset;
        m_FinalBoneMatrices[index] = globalTransformation * offset;
    }
    
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        calculateBoneTransform(node->mChildren[i], globalTransformation, animationTime);
    }
}

glm::mat4 AnimatedModel::aiMatrix4x4ToGlm(const aiMatrix4x4& from) {
    glm::mat4 to;
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}

glm::vec3 AnimatedModel::aiVector3DToGlm(const aiVector3D& vec) {
    return glm::vec3(vec.x, vec.y, vec.z);
}

glm::quat AnimatedModel::aiQuaternionToGlm(const aiQuaternion& pOrientation) {
    return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
}

void AnimatedModel::setVertexBoneDataToDefault(Vertex& vertex) {
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
        vertex.m_BoneIDs[i] = -1;
        vertex.m_Weights[i] = 0.0f;
    }
}

void AnimatedModel::setVertexBoneData(Vertex& vertex, int boneID, float weight) {
    for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
        if (vertex.m_BoneIDs[i] < 0) {
            vertex.m_Weights[i] = weight;
            vertex.m_BoneIDs[i] = boneID;
            break;
        }
    }
}

void AnimatedModel::extractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene) {
    std::cout << "  Processing " << mesh->mNumBones << " bones for mesh" << std::endl;
    
    for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
        int boneID = -1;
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
        
        if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end()) {
            // Check if bone ID would exceed MAX_BONES limit
            if (m_BoneCounter >= 100) {
                std::cout << "WARNING:: Bone ID " << m_BoneCounter << " exceeds MAX_BONES limit (100) for bone: " << boneName << std::endl;
                continue; // Skip this bone
            }
            
            BoneInfo newBoneInfo;
            newBoneInfo.id = m_BoneCounter;
            newBoneInfo.offset = aiMatrix4x4ToGlm(mesh->mBones[boneIndex]->mOffsetMatrix);
            m_BoneInfoMap[boneName] = newBoneInfo;
            boneID = m_BoneCounter;
            m_BoneCounter++;
            
            // Debug: Print foot-related bones
            if (boneName.find("Foot") != std::string::npos || 
                boneName.find("Toe") != std::string::npos ||
                boneName.find("foot") != std::string::npos ||
                boneName.find("toe") != std::string::npos) {
                std::cout << "    Found foot bone: " << boneName << " (ID: " << boneID << ")" << std::endl;
            }
        } else {
            boneID = m_BoneInfoMap[boneName].id;
        }
        
        if (boneID == -1 || boneID >= 100) {
            std::cout << "ERROR:: Invalid bone ID for: " << boneName << std::endl;
            continue;
        }
        
        auto weights = mesh->mBones[boneIndex]->mWeights;
        int numWeights = mesh->mBones[boneIndex]->mNumWeights;
        
        for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
            int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;
            
            if (vertexId >= (int)vertices.size()) {
                std::cout << "ERROR:: Vertex ID " << vertexId << " out of range (max: " << vertices.size() << ")" << std::endl;
                continue;
            }
            
            setVertexBoneData(vertices[vertexId], boneID, weight);
        }
    }
    
    // Validate weights after processing all bones
    int verticesWithZeroWeight = 0;
    int verticesWithInvalidBoneID = 0;
    for (size_t i = 0; i < vertices.size(); ++i) {
        float totalWeight = 0.0f;
        bool hasValidBone = false;
        
        for (int j = 0; j < MAX_BONE_INFLUENCE; ++j) {
            if (vertices[i].m_BoneIDs[j] >= 0) {
                if (vertices[i].m_BoneIDs[j] >= 100) {
                    verticesWithInvalidBoneID++;
                } else {
                    hasValidBone = true;
                }
                totalWeight += vertices[i].m_Weights[j];
            }
        }
        
        if (!hasValidBone || totalWeight < 0.01f) {
            verticesWithZeroWeight++;
            // Debug: Print first few problematic vertices (likely foot vertices)
            if (verticesWithZeroWeight <= 10) {
                std::cout << "WARNING:: Vertex " << i << " has no valid bone weights (total: " << totalWeight << ")" << std::endl;
            }
        }
    }
    
    if (verticesWithZeroWeight > 0) {
        std::cout << "WARNING:: " << verticesWithZeroWeight << " vertices have zero or invalid bone weights!" << std::endl;
    }
    if (verticesWithInvalidBoneID > 0) {
        std::cout << "ERROR:: " << verticesWithInvalidBoneID << " vertices have bone IDs >= 100!" << std::endl;
    }
}

void AnimatedModel::setBoneAdditionalRotation(const std::string& boneName, const glm::quat& additionalRotation) {
    m_AdditionalBoneRotations[boneName] = additionalRotation;
}

void AnimatedModel::clearBoneAdditionalRotation(const std::string& boneName) {
    m_AdditionalBoneRotations.erase(boneName);
}

void AnimatedModel::clearAllBoneAdditionalRotations() {
    m_AdditionalBoneRotations.clear();
}
