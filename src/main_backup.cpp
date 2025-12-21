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

struct camera_t{
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;
    glm::vec3 target;
    
    float yaw;
    float pitch;
    float radius;
    float minRadius;
    float maxRadius;
    float orbitRotateSpeed;
    float orbitZoomSpeed;
    float minOrbitPitch;
    float maxOrbitPitch;
    // auto-orbit settings
    bool enableAutoOrbit;
    float autoOrbitSpeed; // degrees per second
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

// animation timing
float currentTime = 0.0f;
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
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
    
    // Load cart model
#if defined(__linux__) || defined(__APPLE__)
    std::string cart_file = "asset/obj/Car.obj";
#else
    std::string cart_file = "..\\..\\src\\asset\\obj\\Car.obj";
#endif
    
    cartModel = new StaticModel(cart_file);
    
    // Set cart position and scale
    cartMatrix = glm::mat4(1.0f);
    cartMatrix = glm::translate(cartMatrix, glm::vec3(0.0f, 0.0f, -50.0f)); // Position in front of character
    cartMatrix = glm::scale(cartMatrix, glm::vec3(100.0f, 100.0f, 100.0f)); // Scale to appropriate size
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

//////////////////////////////////////////////////////////////////////////
// Parameter setup, 
// You can change any of the settings if you want

void camera_setup(){
    camera.worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    camera.yaw = 90.0f;
    camera.pitch = 10.0f;
    camera.radius = 400.0f;
    camera.minRadius = 150.0f;
    camera.maxRadius = 800.0f;
    camera.orbitRotateSpeed = 60.0f;
    camera.orbitZoomSpeed = 400.0f;
    camera.minOrbitPitch = -80.0f;
    camera.maxOrbitPitch = 80.0f;
    camera.target = glm::vec3(0.0f);
    camera.enableAutoOrbit = true;
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
    std::vector<std::string> faces
    {
        cubemapDir + "right.jpg",
        cubemapDir + "left.jpg",
        cubemapDir + "top.jpg",
        cubemapDir + "bottom.jpg",
        cubemapDir + "front.jpg",
        cubemapDir + "back.jpg"
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
    
    // Update animation
    animatedModel->updateAnimation(currentTime);

    // Auto-orbit camera around target
    if (camera.enableAutoOrbit) {
        float yawDelta = camera.autoOrbitSpeed * deltaTime;
        applyOrbitDelta(yawDelta, 0.0f, 0.0f);
    }
}

void render(){
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // calculate view, projection matrix using new camera system
    glm::mat4 view = glm::lookAt(camera.position + glm::vec3(0.0f, -0.2f, -0.1f), camera.position + camera.front, camera.up);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);

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
    if (cartModel && cartModel->vertices.size() > 0) {
        staticShader->use();
        staticShader->set_uniform_value("model", cartMatrix);
        staticShader->set_uniform_value("view", view);
        staticShader->set_uniform_value("projection", projection);
        
        // Set texture sampler (texture will be set by render function based on material)
        staticShader->set_uniform_value("ourTexture", 0);
        
        // Render model (will handle material switching internally)
        cartModel->render();
        staticShader->release();
    }

    // TODO: Rendering cubemap environment
    // Hint:
    // 1. All the needed things are already set up in cubemap_setup() function.
    // 2. You can use the vertices in cubemapVertices provided in the header/cube.h
    // 3. You can use the cubemapShader to render the cubemap 
    //    (refer to the above code to get an idea of how to use the shader program)

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
    for (auto shader : shaderPrograms) {
        delete shader;
    }
    delete cubemapShader;
    if (staticShader) delete staticShader;

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
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
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
