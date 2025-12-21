#ifndef ENERGY_BEAM_H
#define ENERGY_BEAM_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <random>

class EnergyBeamSystem {
public:
    EnergyBeamSystem(int numBeams = 100);
    ~EnergyBeamSystem();

    void render(float time);
    void setup();
    void setCenter(const glm::vec3& center); // set explosion center point

    unsigned int VAO, VBO;
    int numBeams;
    glm::vec3 explosionCenter; // explosion center position

private:
    struct EnergyBeam {
        glm::vec3 direction;  // shooting direction (unit vector)
        float speed;          // shooting speed
        float offset;         // time offset
        glm::vec3 color;      // RGB color
    };

    std::vector<EnergyBeam> beams;
    void generateBeams();
};

#endif
