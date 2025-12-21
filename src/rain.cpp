#include "header/rain.h"
#include <iostream>

RainSystem::RainSystem(int numDrops) : numRaindrops(numDrops) {
    generateRaindrops();
}

RainSystem::~RainSystem() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void RainSystem::generateRaindrops() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posX(-150.0f, 150.0f);
    std::uniform_real_distribution<float> posY(0.0f, 150.0f);
    std::uniform_real_distribution<float> posZ(-150.0f, 150.0f);
    std::uniform_real_distribution<float> speedDist(20.0f, 40.0f);
    std::uniform_real_distribution<float> offsetDist(0.0f, 10.0f);

    raindrops.clear();
    for (int i = 0; i < numRaindrops; i++) {
        RainDrop drop;
        drop.position = glm::vec3(posX(gen), posY(gen), posZ(gen));
        drop.speed = speedDist(gen);
        drop.offset = offsetDist(gen);
        raindrops.push_back(drop);
    }
}

void RainSystem::setup() {
    // 创建 VAO 和 VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // 上传雨滴数据到 GPU
    glBufferData(GL_ARRAY_BUFFER, raindrops.size() * sizeof(RainDrop),
                 raindrops.data(), GL_STATIC_DRAW);

    // 位置属性 (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RainDrop),
                          (void*)offsetof(RainDrop, position));

    // 速度属性 (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(RainDrop),
                          (void*)offsetof(RainDrop, speed));

    // 偏移属性 (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(RainDrop),
                          (void*)offsetof(RainDrop, offset));

    glBindVertexArray(0);

    std::cout << "Rain system initialized with " << numRaindrops << " raindrops" << std::endl;
    std::cout << "First raindrop position: (" << raindrops[0].position.x << ", "
              << raindrops[0].position.y << ", " << raindrops[0].position.z << ")" << std::endl;
}

void RainSystem::render(float time) {
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, numRaindrops);
    glBindVertexArray(0);
}
