#version 330 core
layout (triangles) in;
// 注意：如果你要產生 5 層殘影，max_vertices 至少要設為 3 * 5 = 15
layout (triangle_strip, max_vertices = 15) out;

in VS_OUT {
    vec3 objectPos;  // 物体空间位置
    vec3 position;   // 世界空间位置
    vec3 normal;     // 世界空间法线
    vec2 texCoord;   // 纹理坐标
} gs_in[];

out vec2 TexCoords;
out float Alpha; // 用來控制殘影透明度

uniform float time;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;  // 當前的模型矩陣

const float PI = 3.14159265359;

// 根據時間重建模型矩陣（重現物體的運動）
mat4 getModelMatrixAtTime(float t) {
    // 重現 main.cpp 中的運動邏輯：
    // degree += 100.0f * deltaTime;
    // Height = 30 * sin(dogTime * 2 * PI / 6) + 50;

    float degreeAtTime = t * 100.0; // 每秒旋轉100度
    float heightAtTime = 30.0 * sin(t * 2.0 * PI / 6.0) + 50.0;

    mat4 m = mat4(1.0);

    // 1. 縮放
    m = mat4(
        100.0, 0.0, 0.0, 0.0,
        0.0, 100.0, 0.0, 0.0,
        0.0, 0.0, 100.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );

    // 2. 平移（Y軸高度）
    mat4 translate = mat4(1.0);
    translate[3] = vec4(0.0, heightAtTime, 0.0, 1.0);
    m = translate * m;

    // 3. 旋轉（繞Y軸）
    float angleRad = radians(degreeAtTime);
    float c = cos(angleRad);
    float s = sin(angleRad);
    mat4 rotate = mat4(
        c, 0.0, s, 0.0,
        0.0, 1.0, 0.0, 0.0,
        -s, 0.0, c, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    m = rotate * m;

    return m;
}

void main() {
    // 設定殘影數量
    int layers = 5;
    float timeLag = 0.05; // 每一層殘影的時間延遲

    // 對每一層殘影進行迴圈
    for(int i = 0; i < layers; i++) {

        // 越後面的層 (i 越大)，時間越早 (time - lag)，Alpha 越低
        float current_time = time - (float(i) * timeLag);

        float current_alpha = (2.0 - (float(i) / float(layers - 1)))/2;

        // 繪製三角形的三個頂點
        for(int j = 0; j < 3; j++) {
            // 1. 獲取該時間點的模型變換矩陣
            mat4 pastModel = getModelMatrixAtTime(current_time);

            // 2. 使用物體空間坐標應用過去的模型矩陣
            vec4 worldPos = pastModel * vec4(gs_in[j].objectPos, 1.0);

            // 3. 將世界空間位置轉換到裁剪空間
            gl_Position = projection * view * worldPos;

            // 4. 傳遞資料
            TexCoords = gs_in[j].texCoord;
            Alpha = current_alpha;

            EmitVertex();
        }
        EndPrimitive(); // 每一層殘影是一個獨立的三角形帶
    }
}