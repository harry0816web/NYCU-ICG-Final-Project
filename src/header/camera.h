#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

struct camera_t {
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

#endif

