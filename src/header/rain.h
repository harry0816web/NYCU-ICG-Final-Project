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
        glm::vec3 position;  // 起始位置
        float speed;         // 下落速度
        float offset;        // 时间偏移
    };

    std::vector<RainDrop> raindrops;
    void generateRaindrops();
};

#endif
