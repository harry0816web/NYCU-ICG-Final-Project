#ifndef SHOCKWAVE_RINGS_H
#define SHOCKWAVE_RINGS_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <random>

struct ShockwaveRing {
    float startRadius;      // 起始半徑
    float speed;            // 擴張速度
    float offset;           // 時間偏移量
    float thickness;        // 環的厚度
    glm::vec3 color;        // RGB 顏色
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
