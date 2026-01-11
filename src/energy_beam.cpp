#include "header/energy_beam.h"
#include <iostream>

EnergyBeamSystem::EnergyBeamSystem(int numBeams)
    : numBeams(numBeams), explosionCenter(glm::vec3(0.0f)) {
    // precompute per-beam attributes before uploading to gpu
    generateBeams();
}

EnergyBeamSystem::~EnergyBeamSystem() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void EnergyBeamSystem::setCenter(const glm::vec3& center) {
    explosionCenter = center;
}

void EnergyBeamSystem::generateBeams() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> speedDist(30.0f, 80.0f);
    std::uniform_real_distribution<float> offsetDist(0.0f, 0.5f);
    std::uniform_real_distribution<float> mixDist(0.0f, 1.0f);

    // define basic colors: red, orange, yellow
    glm::vec3 red(1.0f, 0.0f, 0.0f);
    glm::vec3 orange(1.0f, 0.5f, 0.0f);
    glm::vec3 yellow(1.0f, 1.0f, 0.0f);

    // uniformly distributed directions on sphere surface
    const float PI = 3.14159265359f;
    std::uniform_real_distribution<float> thetaDist(0.0f, 2.0f * PI);
    std::uniform_real_distribution<float> phiDist(0.0f, PI);

    beams.clear();
    for (int i = 0; i < numBeams; i++) {
        EnergyBeam beam;

        // use spherical coordinates to generate uniformly distributed directions
        float theta = thetaDist(gen);
        float phi = phiDist(gen);

        beam.direction = glm::normalize(glm::vec3(
            sin(phi) * cos(theta),
            sin(phi) * sin(theta),
            cos(phi)
        ));

        beam.speed = speedDist(gen);
        beam.offset = offsetDist(gen);

        // randomly mix red, orange, yellow three colors
        float r = mixDist(gen);
        float o = mixDist(gen);
        float y = mixDist(gen);
        float total = r + o + y;
        // normalize weights
        r /= total;
        o /= total;
        y /= total;
        // mix colors
        beam.color = red * r + orange * o + yellow * y;

        beams.push_back(beam);
    }
}

void EnergyBeamSystem::setup() {
    // create VAO and VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // upload energy beam data to GPU
    glBufferData(GL_ARRAY_BUFFER, beams.size() * sizeof(EnergyBeam),
                 beams.data(), GL_STATIC_DRAW);

    // direction attribute (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(EnergyBeam),
                          (void*)offsetof(EnergyBeam, direction));

    // speed attribute (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(EnergyBeam),
                          (void*)offsetof(EnergyBeam, speed));

    // offset attribute (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(EnergyBeam),
                          (void*)offsetof(EnergyBeam, offset));

    // color attribute (location = 3) - RGB vec3
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(EnergyBeam),
                          (void*)offsetof(EnergyBeam, color));

    glBindVertexArray(0);

    std::cout << "Energy beam system initialized with " << numBeams << " beams" << std::endl;
}

void EnergyBeamSystem::render(float time) {
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, numBeams);
    glBindVertexArray(0);
}
