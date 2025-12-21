#include "header/energy_beam.h"
#include <iostream>

EnergyBeamSystem::EnergyBeamSystem(int numBeams)
    : numBeams(numBeams), explosionCenter(glm::vec3(0.0f)) {
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

    // 定義基本顏色：紅、橘、黃
    glm::vec3 red(1.0f, 0.0f, 0.0f);
    glm::vec3 orange(1.0f, 0.5f, 0.0f);
    glm::vec3 yellow(1.0f, 1.0f, 0.0f);

    // 均勻分佈在球面上的方向
    const float PI = 3.14159265359f;
    std::uniform_real_distribution<float> thetaDist(0.0f, 2.0f * PI);
    std::uniform_real_distribution<float> phiDist(0.0f, PI);

    beams.clear();
    for (int i = 0; i < numBeams; i++) {
        EnergyBeam beam;

        // 使用球面坐標產生均勻分佈的方向
        float theta = thetaDist(gen);
        float phi = phiDist(gen);

        beam.direction = glm::normalize(glm::vec3(
            sin(phi) * cos(theta),
            sin(phi) * sin(theta),
            cos(phi)
        ));

        beam.speed = speedDist(gen);
        beam.offset = offsetDist(gen);

        // 隨機混合紅、橘、黃三種顏色
        float r = mixDist(gen);
        float o = mixDist(gen);
        float y = mixDist(gen);
        float total = r + o + y;
        // 正規化權重
        r /= total;
        o /= total;
        y /= total;
        // 混合顏色
        beam.color = red * r + orange * o + yellow * y;

        beams.push_back(beam);
    }
}

void EnergyBeamSystem::setup() {
    // 創建 VAO 和 VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // 上傳能量線數據到 GPU
    glBufferData(GL_ARRAY_BUFFER, beams.size() * sizeof(EnergyBeam),
                 beams.data(), GL_STATIC_DRAW);

    // 方向屬性 (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(EnergyBeam),
                          (void*)offsetof(EnergyBeam, direction));

    // 速度屬性 (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(EnergyBeam),
                          (void*)offsetof(EnergyBeam, speed));

    // 偏移屬性 (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(EnergyBeam),
                          (void*)offsetof(EnergyBeam, offset));

    // 顏色屬性 (location = 3) - RGB vec3
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
