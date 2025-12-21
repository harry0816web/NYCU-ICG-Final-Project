#ifndef SHOCKWAVE_RINGS_H
#define SHOCKWAVE_RINGS_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <random>

struct ShockwaveRing {
    float startRadius;      // initial radius
    float speed;            // expansion speed
    float offset;           // time offset
    float thickness;        // ring thickness
    glm::vec3 color;        // RGB color
};

class ShockwaveRingSystem {
public:
    ShockwaveRingSystem(int numRings);
    ~ShockwaveRingSystem();

    void setCenter(const glm::vec3& center);
    void setup();
    void render(float time);

private:
    void generateRings();

    int numRings;
    glm::vec3 shockwaveCenter;
    std::vector<ShockwaveRing> rings;

    GLuint VAO, VBO;
};

#endif
