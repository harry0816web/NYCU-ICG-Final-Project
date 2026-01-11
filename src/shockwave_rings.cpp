#include "header/shockwave_rings.h"
#include <iostream>

ShockwaveRingSystem::ShockwaveRingSystem(int numRings)
    : numRings(numRings), shockwaveCenter(glm::vec3(0.0f)) {
    // precompute ring attributes so GPU buffer stays static
    generateRings();
}

ShockwaveRingSystem::~ShockwaveRingSystem() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void ShockwaveRingSystem::setCenter(const glm::vec3& center) {
    shockwaveCenter = center;
}

void ShockwaveRingSystem::generateRings() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> speedDist(15.0f, 35.0f);
    std::uniform_real_distribution<float> offsetDist(0.0f, 0.3f);
    std::uniform_real_distribution<float> thicknessDist(0.3f, 0.8f);
    std::uniform_real_distribution<float> mixDist(0.0f, 1.0f);

    // define basic colors: white, light blue, cyan
    glm::vec3 white(1.0f, 1.0f, 1.0f);
    glm::vec3 lightBlue(0.6f, 0.8f, 1.0f);
    glm::vec3 cyan(0.5f, 1.0f, 1.0f);

    rings.clear();
    for (int i = 0; i < numRings; i++) {
        ShockwaveRing ring;

        ring.startRadius = 0.5f;
        ring.speed = speedDist(gen);
        ring.offset = offsetDist(gen);
        ring.thickness = thicknessDist(gen);

        // randomly mix white, light blue, cyan three colors
        float w = mixDist(gen);
        float b = mixDist(gen);
        float c = mixDist(gen);
        float total = w + b + c;
        w /= total;
        b /= total;
        c /= total;
        ring.color = white * w + lightBlue * b + cyan * c;

        rings.push_back(ring);
    }
}

void ShockwaveRingSystem::setup() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, rings.size() * sizeof(ShockwaveRing),
                 rings.data(), GL_STATIC_DRAW);

    // initial radius (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(ShockwaveRing),
                          (void*)offsetof(ShockwaveRing, startRadius));

    // speed (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(ShockwaveRing),
                          (void*)offsetof(ShockwaveRing, speed));

    // offset (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ShockwaveRing),
                          (void*)offsetof(ShockwaveRing, offset));

    // thickness (location = 3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(ShockwaveRing),
                          (void*)offsetof(ShockwaveRing, thickness));

    // color (location = 4)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(ShockwaveRing),
                          (void*)offsetof(ShockwaveRing, color));

    glBindVertexArray(0);

    std::cout << "Shockwave ring system initialized with " << numRings << " rings" << std::endl;
}

void ShockwaveRingSystem::render(float time) {
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, numRings);
    glBindVertexArray(0);
}
