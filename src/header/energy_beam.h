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
    void setCenter(const glm::vec3& center); // 設置爆炸中心點

    unsigned int VAO, VBO;
    int numBeams;
    glm::vec3 explosionCenter; // 爆炸中心位置

private:
    struct EnergyBeam {
        glm::vec3 direction;  // 射出方向（單位向量）
        float speed;          // 射出速度
        float offset;         // 時間偏移
        glm::vec3 color;      // RGB 顏色（紅黃橘混合）
    };

    std::vector<EnergyBeam> beams;
    void generateBeams();
};

#endif
