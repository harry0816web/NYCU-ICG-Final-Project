#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "header/cube.h"
#include "header/animated_model.h"
#include "header/static_model.h"
#include "header/shader.h"
#include "header/stb_image.h"
#include "header/rain.h"
#include "header/cinematic_director.h"
#include "header/camera.h"

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow *window);
unsigned int loadCubemap(std::vector<std::string> &mFileName);

struct material_t{
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float gloss;
};

struct light_t{
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

// settings
int SCR_WIDTH = 800;
int SCR_HEIGHT = 600;

// cube map 
unsigned int cubemapTexture;
unsigned int cubemapVAO, cubemapVBO;

// shader programs 
int shaderProgramIndex = 0;
std::vector<shader_program_t*> shaderPrograms;
shader_program_t* cubemapShader;
shader_program_t* rainShader;
shader_program_t* motionBlurShader = nullptr;

StaticModel* cityModel = nullptr;
glm::mat4 cityMatrix;

light_t light;
material_t material;
camera_t camera;

// animated model
AnimatedModel* animatedModel;
glm::mat4 modelMatrix;

// static model (cart)
StaticModel* cartModel = nullptr;
glm::mat4 cartMatrix;
shader_program_t* staticShader = nullptr;

// rain system
RainSystem* rainSystem;
bool enableRain = true;

// cinematic director
CinematicDirector* cinematicDirector = nullptr;
bool enableCinematic = false;

// animation timing
float currentTime = 0.0f;
float animationStartTime = 0.0f;
bool animationStarted = false;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void model_setup(){
#if defined(__linux__) || defined(__APPLE__)
    std::string fbx_file = "asset/Walking.fbx";
    std::string texture_dir = "asset/texture/";
#else
    std::string fbx_file = "..\\..\\src\\asset\\Walking.fbx";
    std::string texture_dir = "..\\..\\src\\asset\\texture\\";
#endif

    // Load the animated FBX model
    animatedModel = new AnimatedModel(fbx_file);
    
    // Load texture manually (FBX may or may not have embedded texture)
#if defined(__linux__) || defined(__APPLE__)
    animatedModel->loadTexture("asset/texture/rp_eric_rigged_001_dif.jpg");
#else
    animatedModel->loadTexture("..\\..\\src\\asset\\texture\\rp_eric_rigged_001_dif.jpg");
#endif
    
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f, 0.1f, 0.1f));
    // 初始位置：x=150, z=100，朝向右边（绕Y轴旋转-90度，让角色面向+X方向）
    modelMatrix = glm::translate(modelMatrix, glm::vec3(150.0f, 0.0f, 100.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // 朝向右边
    
    // Load cart model
#if defined(__linux__) || defined(__APPLE__)
    std::string cart_file = "asset/obj/Car.obj";
#else
    std::string cart_file = "..\\..\\src\\asset\\obj\\Car.obj";
#endif
    
    cartModel = new StaticModel(cart_file);
    
    // Set cart position and scale
    // 初始位置：在人物左方，往+Z很多（遠離螢幕），往+X一點（往右）
    // OpenGL座標系說明：
    // - X軸：右為正(+X)，左為負(-X)
    // - Y軸：上為正(+Y)，下為負(-Y)
    // - Z軸：前為正(+Z，螢幕外)，後為負(-Z，螢幕裡)
    cartMatrix = glm::mat4(1.0f);
    cartMatrix = glm::translate(cartMatrix, glm::vec3(0.0f, 0.0f, 150.0f)); // Position: X=-20 (往右一點), Z=40 (往+Z很多，遠離螢幕)
    cartMatrix = glm::rotate(cartMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // 旋轉180度，讓車頭朝向螢幕裡面（-Z方向）
    cartMatrix = glm::scale(cartMatrix, glm::vec3(10.0f, 10.0f, 10.0f)); // Scale to appropriate size

    // // Load city model - COMMENTED OUT
    // #if defined(__linux__) || defined(__APPLE__)
    // std::string city_file = "asset/obj/city.obj";
    // #else
    // std::string city_file = "..\\..\\src\\asset\\obj\\city.obj";
    // #endif

    // cityModel = new StaticModel(city_file);

    // // Set city position and scale
    // cityMatrix = glm::mat4(1.0f);
    // cityMatrix = glm::translate(cityMatrix, glm::vec3(0.0f, 0.0f, 0.0f)); // Position at origin
    // cityMatrix = glm::scale(cityMatrix, glm::vec3(5.0f, 5.0f, 5.0f)); // Keep original scale
}

void updateCamera(){
    // Convert spherical orbit definition to cartesian camera position
    float yawRad = glm::radians(camera.yaw);
    float pitchRad = glm::radians(camera.pitch);
    float cosPitch = cos(pitchRad);

    camera.position.x = camera.target.x + camera.radius * cosPitch * cos(yawRad);
    camera.position.y = camera.target.y + camera.radius * sin(pitchRad);
    camera.position.z = camera.target.z + camera.radius * cosPitch * sin(yawRad);

    camera.front = glm::normalize(camera.target - camera.position);
    camera.right = glm::normalize(glm::cross(camera.front, camera.worldUp));
    camera.up = glm::normalize(glm::cross(camera.right, camera.front));
}

void applyOrbitDelta(float yawDelta, float pitchDelta, float radiusDelta) {
    camera.yaw += yawDelta;
    camera.pitch = glm::clamp(camera.pitch + pitchDelta, camera.minOrbitPitch, camera.maxOrbitPitch);
    camera.radius = glm::clamp(camera.radius + radiusDelta, camera.minRadius, camera.maxRadius);
    updateCamera();
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // 允许手动控制相机（固定視角模式）
    // COMMENTED OUT: 暫時註釋掉鏡頭軌跡檢查
    // if (!enableCinematic) {
    {
        glm::vec2 orbitInput(0.0f);
        float zoomInput = 0.0f;

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            orbitInput.x += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            orbitInput.x -= 1.0f;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            orbitInput.y += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            orbitInput.y -= 1.0f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            zoomInput -= 1.0f;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            zoomInput += 1.0f;

        if (orbitInput.x != 0.0f || orbitInput.y != 0.0f || zoomInput != 0.0f) {
            float yawDelta = orbitInput.x * camera.orbitRotateSpeed * deltaTime;
            float pitchDelta = orbitInput.y * camera.orbitRotateSpeed * deltaTime;
            float radiusDelta = zoomInput * camera.orbitZoomSpeed * deltaTime;
            applyOrbitDelta(yawDelta, pitchDelta, radiusDelta);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Parameter setup, 
// You can change any of the settings if you want

void camera_setup(){
    camera.worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    camera.yaw = 90.0f;
    camera.pitch = 10.0f;
    camera.radius = 400.0f;
    camera.minRadius = 0.1f;  // 允許非常靠近物體進行特寫
    camera.maxRadius = 10000.0f;  // 允許很遠的距離
    camera.orbitRotateSpeed = 60.0f;
    camera.orbitZoomSpeed = 400.0f;
    camera.minOrbitPitch = -80.0f;
    camera.maxOrbitPitch = 80.0f;
    camera.target = glm::vec3(0.0f);
    camera.enableAutoOrbit = false;  // Disable auto-orbit (fixed view, no rotation)
    camera.autoOrbitSpeed = 20.0f;

    updateCamera();
}

void light_setup(){
    light.position = glm::vec3(1000.0, 1000.0, 0.0);
    light.ambient = glm::vec3(1.0);
    light.diffuse = glm::vec3(1.0);
    light.specular = glm::vec3(1.0);
}

void material_setup(){
    material.ambient = glm::vec3(0.5);
    material.diffuse = glm::vec3(1.0);
    material.specular = glm::vec3(0.7);
    material.gloss = 50.0;
}

void rain_setup(){
    #if defined(__linux__) || defined(__APPLE__)
        std::string shaderDir = "shaders/";
    #else
        std::string shaderDir = "..\\..\\src\\shaders\\";
    #endif
    
        // setup rain shader with geometry shader
        std::string vpath = shaderDir + "rain.vert";
        std::string gpath = shaderDir + "rain.geom";
        std::string fpath = shaderDir + "rain.frag";
    
        rainShader = new shader_program_t();
        rainShader->create();
        rainShader->add_shader(vpath, GL_VERTEX_SHADER);
        rainShader->add_shader(gpath, GL_GEOMETRY_SHADER);
        rainShader->add_shader(fpath, GL_FRAGMENT_SHADER);
        rainShader->link_shader();
    
        // create rain system
        rainSystem = new RainSystem(1500); // 1500 raindrops
        rainSystem->setup();
    }
    
//////////////////////////////////////////////////////////////////////////

void shader_setup(){
#if defined(__linux__) || defined(__APPLE__)
    std::string shaderDir = "shaders/";
#else
    shaderDir = "shaders\\";
#endif

    std::vector<std::string> shadingMethod = {
        "default", "bling-phong", "gouraud", "metallic", "glass_schlick"
    };

    // Create animated versions of all original shaders
    for(int i=0; i<shadingMethod.size(); i++){
        std::string vpath = shaderDir + "animated_" + shadingMethod[i] + ".vert";
        std::string fpath = shaderDir + shadingMethod[i] + ".frag";

        shader_program_t* shaderProgram = new shader_program_t();
        shaderProgram->create();
        shaderProgram->add_shader(vpath, GL_VERTEX_SHADER);
        shaderProgram->add_shader(fpath, GL_FRAGMENT_SHADER);
        shaderProgram->link_shader();
        shaderPrograms.push_back(shaderProgram);
    }
    
    // Create static model shader (for cart)
    staticShader = new shader_program_t();
    staticShader->create();
    std::string staticVertPath = shaderDir + "static_default.vert";
    std::string staticFragPath = shaderDir + "default.frag";
    staticShader->add_shader(staticVertPath, GL_VERTEX_SHADER);
    staticShader->add_shader(staticFragPath, GL_FRAGMENT_SHADER);
    staticShader->link_shader();

    // motion blur shader
    // Create motion blur shader (for cart with motion blur effect)
    motionBlurShader = new shader_program_t();
    motionBlurShader->create();
    std::string motionBlurVertPath = shaderDir + "motion_blur.vert";
    std::string motionBlurGeomPath = shaderDir + "motion_blur.geom";
    std::string motionBlurFragPath = shaderDir + "motion_blur.frag";
    motionBlurShader->add_shader(motionBlurVertPath, GL_VERTEX_SHADER);
    motionBlurShader->add_shader(motionBlurGeomPath, GL_GEOMETRY_SHADER);
    motionBlurShader->add_shader(motionBlurFragPath, GL_FRAGMENT_SHADER);
    motionBlurShader->link_shader();
}

void cubemap_setup(){
    #if defined(__linux__) || defined(__APPLE__)
        std::string cubemapDir = "asset/texture/skybox/";
        std::string shaderDir = "shaders/";
    #else
        std::string cubemapDir = "..\\..\\src\\asset\\texture\\skybox\\";
        std::string shaderDir = "..\\..\\src\\shaders\\";
    #endif
    
        // setup texture for cubemap
        // Order: +X (right), -X (left), +Y (top), -Y (bottom), +Z (back), -Z (front)
        std::vector<std::string> faces
        {
            cubemapDir + "px.png",
            cubemapDir + "nx.png",
            cubemapDir + "py.png",
            cubemapDir + "ny.png",
            cubemapDir + "pz.png",
            cubemapDir + "nz.png"
        };
        cubemapTexture = loadCubemap(faces);   
    
        // setup shader for cubemap
        std::string vpath = shaderDir + "cubemap.vert";
        std::string fpath = shaderDir + "cubemap.frag";
        
        cubemapShader = new shader_program_t();
        cubemapShader->create();
        cubemapShader->add_shader(vpath, GL_VERTEX_SHADER);
        cubemapShader->add_shader(fpath, GL_FRAGMENT_SHADER);
        cubemapShader->link_shader();
    
        glGenVertexArrays(1, &cubemapVAO);
        glGenBuffers(1, &cubemapVBO);
        glBindVertexArray(cubemapVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cubemapVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubemapVertices), &cubemapVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glBindVertexArray(0);
    }

void setup(){
    // initialize shader model camera light material
    light_setup();
    model_setup();
    shader_setup();
    camera_setup();
    cubemap_setup();
    material_setup();
    rain_setup();
    
    // Initialize cinematic director (pass animatedModel for head rotation control)
    cinematicDirector = new CinematicDirector(camera, modelMatrix, cartMatrix, animatedModel);
    std::cout << "Cinematic Director initialized. Press 'C' to start/stop cinematic mode." << std::endl;

    // enable depth test, face culling
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    // debug - but filter out expected uniform location errors
    // glEnable(GL_DEBUG_OUTPUT);
    // glDebugMessageCallback([](  GLenum source, GLenum type, GLuint id, GLenum severity, 
    //                             GLsizei length, const GLchar* message, const void* userParam) {

    // std::cerr << "GL CALLBACK: " << (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "") 
    //           << "type = " << type 
    //           << ", severity = " << severity 
    //           << ", message = " << message << std::endl;
    // }, nullptr);
}

void update(){
    // Update timing
    currentTime = glfwGetTime();
    deltaTime = currentTime - lastFrame;
    lastFrame = currentTime;
    
    // 每秒輸出相機位置和lookat，用於記錄關鍵幀（一開始就輸出，不用按C）
    static float lastCameraOutputTime = -1.0f;
    if (currentTime - lastCameraOutputTime >= 1.0f || lastCameraOutputTime < 0.0f) {
        // 計算實際的lookat點：在orbit系統中，lookat點 = position + front * radius = target
        // 但為了確保正確，我們直接計算：position + front * radius
        glm::vec3 actualLookAt = camera.position + camera.front * camera.radius;
        std::cout << "Camera Keyframe: Time=" << (int)currentTime << "s, Position=(" 
                  << camera.position.x << ", " << camera.position.y << ", " << camera.position.z 
                  << "), LookAt=(" << actualLookAt.x << ", " << actualLookAt.y << ", " << actualLookAt.z << ")" << std::endl;
        lastCameraOutputTime = currentTime;
    }
    
    // Update animation (use relative time if animation has started)
    float animationTime = animationStarted ? (currentTime - animationStartTime) : 0.0f;
    
    // 計算用於動畫的時間（如果翻滾已開始，停止走路動畫）
    float animationTimeForModel = animationTime;
    if (cinematicDirector && animationStarted) {
        float rollStartTime = cinematicDirector->GetRollStartTime();
        if (rollStartTime >= 0.0f && animationTime >= rollStartTime) {
            // 翻滾開始後，保持動畫時間在翻滾開始時（停止走路動畫）
            animationTimeForModel = rollStartTime;
        }
    }
    
    // 使用限制後的動畫時間更新模型動畫（停止走路動畫）
    animatedModel->updateAnimation(animationTimeForModel);

    // Update character and cart movement based on animation time
    if (cinematicDirector) {
        if (animationStarted) {
            // 動畫開始後，使用原始動畫時間更新位置（不限制，讓翻滾可以繼續）
            cinematicDirector->UpdateCharacterMovement(animationTime);
            cinematicDirector->UpdateCartMovement(animationTime);
            cinematicDirector->UpdateHeadRotation(animationTime);
            cinematicDirector->UpdateCameraWithTime(animationTime); // 更新相機
            // 注意：m_CharacterModel 和 m_CartModel 是引用，直接更新了 modelMatrix 和 cartMatrix
            // 所以不需要再次賦值
        } else {
            // 動畫未開始時，確保使用初始位置（與 model_setup 中設置的一致）
            // 這樣動畫開始時會從初始位置開始
            cinematicDirector->UpdateCharacterMovement(0.0f);
            cinematicDirector->UpdateCartMovement(0.0f);
            // 注意：m_CharacterModel 和 m_CartModel 是引用，直接更新了 modelMatrix 和 cartMatrix
        }
    }
}

void render(){
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // calculate view, projection matrix using new camera system
    glm::mat4 view = glm::lookAt(camera.position + glm::vec3(0.0f, -0.2f, -0.1f), camera.position + camera.front, camera.up);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);

    // rain
    if (enableRain) {
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glLineWidth(2.0f);

        rainShader->use();
        rainShader->set_uniform_value("view", view);
        rainShader->set_uniform_value("projection", projection);
        rainShader->set_uniform_value("time", currentTime);
        rainShader->set_uniform_value("rainLength", 8.0f);

        rainSystem->render(currentTime);
        rainShader->release();

        glLineWidth(1.0f);
        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
    }

    // Render animated model
    if (shaderProgramIndex < shaderPrograms.size()) {
        // Set matrix for view, projection, model transformation
        shaderPrograms[shaderProgramIndex]->use();
        
        // Common uniforms for all shaders
        shaderPrograms[shaderProgramIndex]->set_uniform_value("model", modelMatrix);
        shaderPrograms[shaderProgramIndex]->set_uniform_value("view", view);
        shaderPrograms[shaderProgramIndex]->set_uniform_value("projection", projection);
        shaderPrograms[shaderProgramIndex]->set_uniform_value("viewPos", camera.position);
        
        // todo1-1: Set lighting and material uniforms for all shaders
        // use shader_program_t::set_uniform_value() to set different types of uniforms
        shaderPrograms[shaderProgramIndex]->set_uniform_value("lightPos", light.position);
        shaderPrograms[shaderProgramIndex]->set_uniform_value("lightAmbient", light.ambient);
        shaderPrograms[shaderProgramIndex]->set_uniform_value("lightDiffuse", light.diffuse);
        shaderPrograms[shaderProgramIndex]->set_uniform_value("lightSpecular", light.specular);
        
        shaderPrograms[shaderProgramIndex]->set_uniform_value("materialAmbient", material.ambient);
        shaderPrograms[shaderProgramIndex]->set_uniform_value("materialDiffuse", material.diffuse);
        shaderPrograms[shaderProgramIndex]->set_uniform_value("materialSpecular", material.specular);
        shaderPrograms[shaderProgramIndex]->set_uniform_value("materialShininess", material.gloss);

        // todo1-2: Set cubemap sampler for metallic and glass shaders to interact with the skybox
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        GLint skyboxLoc = glGetUniformLocation(shaderPrograms[shaderProgramIndex]->get_program_id(), "skybox");
        if (skyboxLoc != -1) {
            glUniform1i(skyboxLoc, 1);
        }

        // todo1-3: Set metallic shader specific uniforms
        if (shaderProgramIndex == 3) {
            shaderPrograms[shaderProgramIndex]->set_uniform_value("bias", 0.2f);
            shaderPrograms[shaderProgramIndex]->set_uniform_value("alpha", 0.4f);
            shaderPrograms[shaderProgramIndex]->set_uniform_value("lightIntensity", 1.0f);
        }

        // todo1-4: Set glass shader specific uniforms
        if (shaderProgramIndex == 4) {
            float eta = 1.0f / 1.52f;  // n1/n2
            shaderPrograms[shaderProgramIndex]->set_uniform_value("eta", eta);
        }
        
        // TODO: Set uniform value for each shader program
        // Set texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, animatedModel->texture);
        shaderPrograms[shaderProgramIndex]->set_uniform_value("ourTexture", 0);
        
        // Specifying texture & cubemap sampler for shader program
        
        // Set bone matrices for animation (only for animated shaders)
        // The animated shaders have "animated_" prefix, so we know they support bone matrices
        // For original shaders without bone support, skip this to avoid GL errors
        std::vector<std::string> shadingMethod = {
            "default", "bling-phong", "gouraud", "metallic", "glass_schlick"
        };
        
        // Only set bone matrices for animated shaders (we know these have the uniforms)
        if (shaderProgramIndex < shadingMethod.size()) {
            // Check if the shader has bone matrices uniform before setting it
            GLint boneMatricesLocation = glGetUniformLocation(shaderPrograms[shaderProgramIndex]->get_program_id(), "finalBonesMatrices");
            if (boneMatricesLocation != -1) {
                // Set bone matrices - use actual bone count (up to 200)
                size_t numBones = std::min((size_t)200, animatedModel->m_FinalBoneMatrices.size());
                for (unsigned int i = 0; i < numBones; ++i) {
                    std::string name = "finalBonesMatrices[" + std::to_string(i) + "]";
                    shaderPrograms[shaderProgramIndex]->set_uniform_value(name.c_str(), animatedModel->m_FinalBoneMatrices[i]);
                }
            }
        }
        
        animatedModel->render();
        shaderPrograms[shaderProgramIndex]->release();
    }

    // Render cart (static model)
    // Render cart with motion blur effect
    if (cartModel && cartModel->vertices.size() > 0) {
        // 启用混合模式（用于半透明残影）
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // 使用 motion blur shader
        motionBlurShader->use();
        motionBlurShader->set_uniform_value("model", cartMatrix);
        motionBlurShader->set_uniform_value("view", view);
        motionBlurShader->set_uniform_value("projection", projection);
        
        // 使用动画时间而不是全局时间
        float animationTime = animationStarted ? (currentTime - animationStartTime) : 0.0f;
        motionBlurShader->set_uniform_value("time", animationTime);
        
        // Render model (will handle material switching and texture binding internally)
        cartModel->render();
        motionBlurShader->release();
        
        // 关闭混合模式
        glDisable(GL_BLEND);
    }

    // Render city (static model)
    if (cityModel && cityModel->vertices.size() > 0) {
        staticShader->use();
        staticShader->set_uniform_value("model", cityMatrix);
        staticShader->set_uniform_value("view", view);
        staticShader->set_uniform_value("projection", projection);

        // Set texture sampler (texture will be set by render function based on material)
        staticShader->set_uniform_value("ourTexture", 0);

        // Render model (will handle material switching internally)
        cityModel->render();
        staticShader->release();
    }

    // TODO: Rendering cubemap environment
    // Hint:
    // 1. All the needed things are already set up in cubemap_setup() function.
    // 2. You can use the vertices in cubemapVertices provided in the header/cube.h
    // 3. You can use the cubemapShader to render the cubemap 
    //    (refer to the above code to get an idea of how to use the shader program)

    // Skybox rendering enabled
    // todo1-5: render cubemap as background
    // set depth function to LEQUAL to make sure cubemap is behind objects
    glDepthFunc(GL_LEQUAL);
    
    // use cubemapShader to render cubemap and set uniforms
    cubemapShader->use();
    cubemapShader->set_uniform_value("view", view);
    cubemapShader->set_uniform_value("projection", projection);
    
    // bind VAO and texture to render cubemap
    glBindVertexArray(cubemapVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    GLint cubemapSkyboxLoc = glGetUniformLocation(cubemapShader->get_program_id(), "skybox");
    if (cubemapSkyboxLoc != -1) {
        glUniform1i(cubemapSkyboxLoc, 0);
    }
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    cubemapShader->release();
    
    // Change back to default depth function
    glDepthFunc(GL_LESS);
}

int main() {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "HW3-Animated Model", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSwapInterval(1);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // set viewport
    glfwGetFramebufferSize(window, &SCR_WIDTH, &SCR_HEIGHT);
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // setup texture, model, shader ...e.t.c
    setup();
    
    // render loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        update(); 
        render(); 
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup
    delete animatedModel;
    if (cartModel) delete cartModel;
    if (cityModel) delete cityModel;
    for (auto shader : shaderPrograms) {
        delete shader;
    }
    delete cubemapShader;
    if (staticShader) delete staticShader;
    if (cinematicDirector) delete cinematicDirector;
    if (motionBlurShader) delete motionBlurShader;

    delete rainShader;
    delete rainSystem;
    glfwTerminate();
    return 0;
}

// add key callback
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // shader program selection
    if (key == GLFW_KEY_0 && (action == GLFW_REPEAT || action == GLFW_PRESS)) 
        shaderProgramIndex = 0;
    if (key == GLFW_KEY_1 && (action == GLFW_REPEAT || action == GLFW_PRESS)) 
        shaderProgramIndex = 1;
    if (key == GLFW_KEY_2 && (action == GLFW_REPEAT || action == GLFW_PRESS)) 
        shaderProgramIndex = 2;
    if (key == GLFW_KEY_3 && action == GLFW_PRESS)
        shaderProgramIndex = 3;
    if (key == GLFW_KEY_4 && action == GLFW_PRESS)
        shaderProgramIndex = 4;
    // 按 R 键切换雨滴显示
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        enableRain = !enableRain;
        std::cout << "Rain effect: " << (enableRain ? "ON" : "OFF") << std::endl;
    }
    
    // 按 C 键开始动画
    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        if (!animationStarted) {
            animationStarted = true;
            animationStartTime = currentTime;
            if (cinematicDirector) {
                cinematicDirector->Start(); // 啟動cinematic director
            }
            std::cout << "Animation started! (Press C again to restart)" << std::endl;
        } else {
            // 重新开始动画
            animationStartTime = currentTime;
            if (cinematicDirector) {
                cinematicDirector->Stop();
                cinematicDirector->Start(); // 重新啟動cinematic director
            }
            std::cout << "Animation restarted!" << std::endl;
        }
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

// loading cubemap texture
unsigned int loadCubemap(std::vector<std::string>& faces)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        stbi_set_flip_vertically_on_load(false);
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return texture;
}
