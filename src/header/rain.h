#ifndef RAIN_H
#define RAIN_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <random>

class RainSystem {
public:
    RainSystem(int numDrops = 2000);
    ~RainSystem();

    void render(float time);
    void setup();

    unsigned int VAO, VBO;
    int numRaindrops;

private:
    struct RainDrop {
        glm::vec3 position;  // initial position
        float speed;         // fall speed
        float offset;        // time offset
    };

    std::vector<RainDrop> raindrops;
    void generateRaindrops();
};

#endif
