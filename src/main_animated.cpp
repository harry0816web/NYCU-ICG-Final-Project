#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <iomanip>


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
#include "header/shockwave_rings.h"
#include "header/energy_beam.h"

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow *window);
unsigned int loadCubemap(std::vector<std::string> &mFileName);

// energy beam system
EnergyBeamSystem* energyBeamSystem = nullptr;
shader_program_t* energyBeamShader = nullptr;
bool enableEnergyBeam = false;
float energyBeamStartTime = -1.0f;

// shockwave rings system
ShockwaveRingSystem* shockwaveSystem = nullptr;
shader_program_t* shockwaveShader = nullptr;
bool enableShockwave = false;
float shockwaveStartTime = -1.0f;

void energy_beam_setup(){
    #if defined(__linux__) || defined(__APPLE__)
        std::string shaderDir = "shaders/";
    #else
        std::string shaderDir = "..\\..\\src\\shaders\\";
    #endif

    std::string vpath = shaderDir + "energy_beam.vert";
    std::string gpath = shaderDir + "energy_beam.geom";
    std::string fpath = shaderDir + "energy_beam.frag";

    energyBeamShader = new shader_program_t();
    energyBeamShader->create();
    energyBeamShader->add_shader(vpath, GL_VERTEX_SHADER);
    energyBeamShader->add_shader(gpath, GL_GEOMETRY_SHADER);
    energyBeamShader->add_shader(fpath, GL_FRAGMENT_SHADER);
    energyBeamShader->link_shader();

    energyBeamSystem = new EnergyBeamSystem(150);
    energyBeamSystem->setup();

    std::cout << "Energy beam system setup complete" << std::endl;
}

void shockwave_setup(){
    #if defined(__linux__) || defined(__APPLE__)
        std::string shaderDir = "shaders/";
    #else
        std::string shaderDir = "..\\..\\src\\shaders\\";
    #endif

    std::string vpath = shaderDir + "shockwave.vert";
    std::string gpath = shaderDir + "shockwave.geom";
    std::string fpath = shaderDir + "shockwave.frag";

    shockwaveShader = new shader_program_t();
    shockwaveShader->create();
    shockwaveShader->add_shader(vpath, GL_VERTEX_SHADER);
    shockwaveShader->add_shader(gpath, GL_GEOMETRY_SHADER);
    shockwaveShader->add_shader(fpath, GL_FRAGMENT_SHADER);
    shockwaveShader->link_shader();

    shockwaveSystem = new ShockwaveRingSystem(20);
    shockwaveSystem->setup();

    std::cout << "Shockwave ring system setup complete" << std::endl;
}

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
shader_program_t* explodeShader = nullptr;
shader_program_t* burningShader = nullptr;

// explode effect
bool enableExplode = false;
float explodeStartTime = -1.0f;
float explodeDuration = 2.0f;  // explosion animation duration
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
    // initial position: x=150, z=100, facing right (rotate -90 degrees around Y axis to make character face +X direction)
    modelMatrix = glm::translate(modelMatrix, glm::vec3(150.0f, 0.0f, 100.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // facing right
    
    // Load cart model
#if defined(__linux__) || defined(__APPLE__)
    std::string cart_file = "asset/obj/Car.obj";
#else
    std::string cart_file = "..\\..\\src\\asset\\obj\\Car.obj";
#endif
    
    cartModel = new StaticModel(cart_file);
    
    // Set cart position and scale
    // initial position: to the left of character, far in +Z direction (away from screen), slightly in +X direction (to the right)
    // OpenGL coordinate system:
    // - X axis: right is positive (+X), left is negative (-X)
    // - Y axis: up is positive (+Y), down is negative (-Y)
    // - Z axis: forward is positive (+Z, out of screen), backward is negative (-Z, into screen)
    cartMatrix = glm::mat4(1.0f);
    cartMatrix = glm::translate(cartMatrix, glm::vec3(0.0f, 0.0f, 150.0f)); // position: X=-20 (slightly to the right), Z=40 (far in +Z direction, away from screen)
    cartMatrix = glm::rotate(cartMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // rotate 180 degrees to make car face inward (toward -Z direction)
    cartMatrix = glm::scale(cartMatrix, glm::vec3(10.0f, 10.0f, 10.0f)); // Scale to appropriate size

    // Load city model - COMMENTED OUT
    #if defined(__linux__) || defined(__APPLE__)
    std::string city_file = "asset/obj/city.obj";
    #else
    std::string city_file = "..\\..\\src\\asset\\obj\\city.obj";
    #endif

    cityModel = new StaticModel(city_file);

    // Set city position and scale
    cityMatrix = glm::mat4(1.0f);
    cityMatrix = glm::translate(cityMatrix, glm::vec3(0.0f, 0.0f, 0.0f)); // Position at origin
    cityMatrix = glm::scale(cityMatrix, glm::vec3(5.0f, 5.0f, 5.0f)); // Keep original scale
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

    // allow manual camera control (fixed view mode)
    // commented out: temporarily disable camera trajectory check
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
    camera.minRadius = 0.1f;  // allow very close to objects for close-up shots
    camera.maxRadius = 10000.0f;  // allow very far distances
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

    // Create explode shader (for animated model with geometry shader)
    explodeShader = new shader_program_t();
    explodeShader->create();
    std::string explodeVertPath = shaderDir + "animated_explode.vert";
    std::string explodeGeomPath = shaderDir + "animated_explode.geom";
    std::string explodeFragPath = shaderDir + "animated_explode.frag";
    explodeShader->add_shader(explodeVertPath, GL_VERTEX_SHADER);
    explodeShader->add_shader(explodeGeomPath, GL_GEOMETRY_SHADER);
    explodeShader->add_shader(explodeFragPath, GL_FRAGMENT_SHADER);
    explodeShader->link_shader();

    // Create burning shader
    burningShader = new shader_program_t();
    burningShader->create();
    std::string burningVertPath = shaderDir + "burning.vert";
    std::string burningGeomPath = shaderDir + "burning.geom";
    std::string burningFragPath = shaderDir + "burning.frag";
    burningShader->add_shader(burningVertPath, GL_VERTEX_SHADER);
    burningShader->add_shader(burningGeomPath, GL_GEOMETRY_SHADER);
    burningShader->add_shader(burningFragPath, GL_FRAGMENT_SHADER);
    burningShader->link_shader();
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
    
    // remove duplicate camera output (unified output by cinematic_director.cpp)
    
    // Update animation (use relative time if animation has started)
    float animationTime = animationStarted ? (currentTime - animationStartTime) : 0.0f;
    
    // calculate animation time (if rolling has started, stop walking animation)
    float animationTimeForModel = animationTime;
    if (cinematicDirector && animationStarted) {
        float rollStartTime = cinematicDirector->GetRollStartTime();
        if (rollStartTime >= 0.0f && animationTime >= rollStartTime) {
            // after rolling starts, keep animation time at roll start time (stop walking animation)
            animationTimeForModel = rollStartTime;
        }
    }
    
    // update model animation with limited animation time (stop walking animation)
    animatedModel->updateAnimation(animationTimeForModel);

    // Update character and cart movement based on animation time
    if (cinematicDirector) {
        if (animationStarted) {
            // after animation starts, use original animation time to update positions (no limit, allow rolling to continue)
            cinematicDirector->UpdateCharacterMovement(animationTime);
            cinematicDirector->UpdateCartMovement(animationTime);
            cinematicDirector->UpdateHeadRotation(animationTime);
            cinematicDirector->UpdateCameraWithTime(animationTime); // update camera
            
            // check if collision occurred and trigger shockwave and energy_beam
            // use static variable to track if already triggered, avoid duplicate triggers
            static float lastCollisionCheckTime = -1.0f;
            float collisionTime = cinematicDirector->GetCollisionTime();
            
            // if collision time changed (new collision occurred), trigger effects
            if (collisionTime >= 0.0f && collisionTime != lastCollisionCheckTime) {
                enableShockwave = true;
                enableEnergyBeam = true;
                shockwaveStartTime = currentTime;
                energyBeamStartTime = currentTime;
                
                // set shockwave and energy_beam center point to cart center
                // cart model matrix: translate(pos) * rotate * scale(10), so cartMatrix[3] directly contains position
                glm::vec3 cartCenter = glm::vec3(cartMatrix[3]);
                if (shockwaveSystem) {
                    shockwaveSystem->setCenter(cartCenter);
                    std::cout << "Shockwave system center set to cart center: (" << cartCenter.x << ", " << cartCenter.y << ", " << cartCenter.z << ")" << std::endl;
                } else {
                    std::cout << "WARNING: shockwaveSystem is null!" << std::endl;
                }
                if (energyBeamSystem) {
                    energyBeamSystem->setCenter(cartCenter);
                    std::cout << "Energy beam system center set to cart center: (" << cartCenter.x << ", " << cartCenter.y << ", " << cartCenter.z << ")" << std::endl;
                } else {
                    std::cout << "WARNING: energyBeamSystem is null!" << std::endl;
                }
                
                lastCollisionCheckTime = collisionTime;
                std::cout << "Collision detected! Time: " << collisionTime << "s, Shockwave and Energy Beam effects started at cart center: (" 
                          << cartCenter.x << ", " << cartCenter.y << ", " << cartCenter.z << ")" << std::endl;
            }
            
            // check if rolling finished and trigger explode and burning effects
            if (cinematicDirector->IsRollFinished() && !enableExplode) {
                enableExplode = true;
                explodeStartTime = currentTime;
                std::cout << "Explode effect started! Burning effect will also be triggered at time: " << currentTime << "s" << std::endl;
                if (!burningShader) {
                    std::cout << "WARNING: burningShader is null! Burning effect may not render." << std::endl;
                }
            }
        } else {
            // when animation hasn't started, ensure using initial positions (consistent with model_setup)
            // so animation will start from initial position
            cinematicDirector->UpdateCharacterMovement(0.0f);
            cinematicDirector->UpdateCartMovement(0.0f);
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

    // Render energy beams from cart center
    if (enableEnergyBeam && energyBeamSystem && energyBeamShader) {
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glLineWidth(3.5f);

        // use cart center as center point
        // cart model matrix: translate(pos) * rotate * scale(10), so cartMatrix[3] directly contains position
        glm::vec3 cartCenter = glm::vec3(cartMatrix[3]);
        cartCenter.y += 12.5f; // slightly raise
        energyBeamSystem->setCenter(cartCenter);

        energyBeamShader->use();
        energyBeamShader->set_uniform_value("view", view);
        energyBeamShader->set_uniform_value("projection", projection);

        float beamTime = currentTime - energyBeamStartTime;
        energyBeamShader->set_uniform_value("time", beamTime);
        energyBeamShader->set_uniform_value("explosionCenter", cartCenter);
        energyBeamShader->set_uniform_value("beamLength", 20.0f);

        energyBeamSystem->render(beamTime);
        energyBeamShader->release();

        glLineWidth(1.0f);
        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
    }

    // Render shockwave rings from cart center
    if (enableShockwave && shockwaveSystem && shockwaveShader) {
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glLineWidth(2.5f);

        // use cart center as center point
        // cart model matrix: translate(pos) * rotate * scale(10), so cartMatrix[3] directly contains position
        glm::vec3 cartCenter = glm::vec3(cartMatrix[3]);
        cartCenter.y = 3.0f; // set at ground height
        shockwaveSystem->setCenter(cartCenter);

        shockwaveShader->use();
        shockwaveShader->set_uniform_value("view", view);
        shockwaveShader->set_uniform_value("projection", projection);

        float shockwaveTime = currentTime - shockwaveStartTime;
        shockwaveShader->set_uniform_value("time", shockwaveTime);
        shockwaveShader->set_uniform_value("shockwaveCenter", cartCenter);

        shockwaveSystem->render(shockwaveTime);
        shockwaveShader->release();

        glLineWidth(1.0f);
        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
    }

    // Render animated model
    // determine whether to use explode effect
    shader_program_t* currentShader = nullptr;
    float explodeStrength = 0.0f;
    
    if (enableExplode && explodeShader) {
        currentShader = explodeShader;
        // calculate explode strength (0.0 to 1.0+)
        float timeSinceExplode = currentTime - explodeStartTime;
        explodeStrength = timeSinceExplode / explodeDuration;
        // no upper limit, allow explosion to continue spreading
    } else if (shaderProgramIndex < shaderPrograms.size()) {
        currentShader = shaderPrograms[shaderProgramIndex];
    }
    
    if (currentShader) {
        // Set matrix for view, projection, model transformation
        currentShader->use();
        
        // Common uniforms for all shaders
        currentShader->set_uniform_value("model", modelMatrix);
        currentShader->set_uniform_value("view", view);
        currentShader->set_uniform_value("projection", projection);
        currentShader->set_uniform_value("viewPos", camera.position);
        
        // if explode shader, set additional uniforms
        if (enableExplode && explodeShader && currentShader == explodeShader) {
            currentShader->set_uniform_value("time", currentTime);
            currentShader->set_uniform_value("explodeStrength", explodeStrength);
        } else {
            // lighting and material uniforms for non-explode shaders
            currentShader->set_uniform_value("lightPos", light.position);
            currentShader->set_uniform_value("lightAmbient", light.ambient);
            currentShader->set_uniform_value("lightDiffuse", light.diffuse);
            currentShader->set_uniform_value("lightSpecular", light.specular);
            
            currentShader->set_uniform_value("materialAmbient", material.ambient);
            currentShader->set_uniform_value("materialDiffuse", material.diffuse);
            currentShader->set_uniform_value("materialSpecular", material.specular);
            currentShader->set_uniform_value("materialShininess", material.gloss);

            // Set cubemap sampler for metallic and glass shaders
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
            GLint skyboxLoc = glGetUniformLocation(currentShader->get_program_id(), "skybox");
            if (skyboxLoc != -1) {
                glUniform1i(skyboxLoc, 1);
            }

            // Set metallic shader specific uniforms
            if (shaderProgramIndex == 3) {
                currentShader->set_uniform_value("bias", 0.2f);
                currentShader->set_uniform_value("alpha", 0.4f);
                currentShader->set_uniform_value("lightIntensity", 1.0f);
            }

            // Set glass shader specific uniforms
            if (shaderProgramIndex == 4) {
                float eta = 1.0f / 1.52f;
                currentShader->set_uniform_value("eta", eta);
            }
        }
        
        // Set texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, animatedModel->texture);
        currentShader->set_uniform_value("ourTexture", 0);
        
        // Set bone matrices for animation
        GLint boneMatricesLocation = glGetUniformLocation(currentShader->get_program_id(), "finalBonesMatrices");
        if (boneMatricesLocation != -1) {
            size_t numBones = std::min((size_t)200, animatedModel->m_FinalBoneMatrices.size());
            for (unsigned int i = 0; i < numBones; ++i) {
                std::string name = "finalBonesMatrices[" + std::to_string(i) + "]";
                currentShader->set_uniform_value(name.c_str(), animatedModel->m_FinalBoneMatrices[i]);
            }
        }
        
        animatedModel->render();
        currentShader->release();
    }

    // Render cart (static model)
    // Render cart with motion blur effect
    if (cartModel && cartModel->vertices.size() > 0) {
        // enable blend mode (for semi-transparent motion trails)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // use motion blur shader
        motionBlurShader->use();
        motionBlurShader->set_uniform_value("model", cartMatrix);
        motionBlurShader->set_uniform_value("view", view);
        motionBlurShader->set_uniform_value("projection", projection);
        
        // use animation time instead of global time
        float animationTime = animationStarted ? (currentTime - animationStartTime) : 0.0f;
        motionBlurShader->set_uniform_value("time", animationTime);
        
        // Render model (will handle material switching and texture binding internally)
        cartModel->render();
        motionBlurShader->release();
        
        // disable blend mode
        glDisable(GL_BLEND);
    }

    // Render burning effect on cart if collided (synced with explode)
    if (enableExplode && explodeStartTime >= 0.0f && burningShader && cartModel && cartModel->vertices.size() > 0) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        burningShader->use();
        burningShader->set_uniform_value("model", cartMatrix);
        burningShader->set_uniform_value("view", view);
        burningShader->set_uniform_value("projection", projection);
        burningShader->set_uniform_value("time", currentTime);
        burningShader->set_uniform_value("carCollisionTime", explodeStartTime); // Sync with explode start time
        
        cartModel->render();
        burningShader->release();
        
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
    // rain_setup();
    energy_beam_setup();
    shockwave_setup();
    
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
    if (explodeShader) delete explodeShader;
    if (cartModel) delete cartModel;
    if (cityModel) delete cityModel;
    if (burningShader) delete burningShader;
    for (auto shader : shaderPrograms) {
        delete shader;
    }
    delete cubemapShader;
    if (staticShader) delete staticShader;
    if (cinematicDirector) delete cinematicDirector;
    if (motionBlurShader) delete motionBlurShader;
    if (energyBeamShader) delete energyBeamShader;
    if (energyBeamSystem) delete energyBeamSystem;
    if (shockwaveShader) delete shockwaveShader;
    if (shockwaveSystem) delete shockwaveSystem;

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
    // press R key to toggle rain display
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        enableRain = !enableRain;
        std::cout << "Rain effect: " << (enableRain ? "ON" : "OFF") << std::endl;
    }
    
    // press C key to start animation
    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        if (!animationStarted) {
            animationStarted = true;
            animationStartTime = currentTime;
            if (cinematicDirector) {
                cinematicDirector->Start(); // start cinematic director
            }
            std::cout << "Animation started! (Press C again to restart)" << std::endl;
        } else {
            // restart animation
            animationStartTime = currentTime;
            if (cinematicDirector) {
                cinematicDirector->Stop();
                cinematicDirector->Start(); // restart cinematic director
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
