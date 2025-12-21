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
    // 重現 cinematic_director.cpp 中 UpdateCartMovement 的運動邏輯
    // 6秒前：靜止在 (0, 0, 150)
    // 6-10秒：從 (0, 0, 150) 移動到 (0, 0, 70)
    // 10秒後：靜止在 (0, 0, 70)
    
    float cartMoveStartTime = 6.0;
    float cartMoveDuration = 4.0; // 4秒完成移動
    vec3 startPos = vec3(0.0, 0.0, 150.0);
    vec3 endPos = vec3(0.0, 0.0, 70.0);
    
    vec3 currentPos;
    if (t < cartMoveStartTime) {
        // 6秒前：保持初始位置
        currentPos = startPos;
    } else if (t > cartMoveStartTime + cartMoveDuration) {
        // 10秒後：保持結束位置
        currentPos = endPos;
    } else {
        // 6-10秒：線性插值移動
        float progress = (t - cartMoveStartTime) / cartMoveDuration;
        // 使用平滑函數（與 C++ 代碼一致）
        float smoothProgress = progress * progress * (3.0 - 2.0 * progress);
        currentPos = mix(startPos, endPos, smoothProgress);
    }
    
    mat4 m = mat4(1.0);
    
    // 1. 縮放（10倍）
    m = mat4(
        10.0, 0.0, 0.0, 0.0,
        0.0, 10.0, 0.0, 0.0,
        0.0, 0.0, 10.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    
    // 2. 旋轉（繞Y軸180度）
    float angleRad = radians(180.0);
    float c = cos(angleRad);
    float s = sin(angleRad);
    mat4 rotate = mat4(
        c, 0.0, s, 0.0,
        0.0, 1.0, 0.0, 0.0,
        -s, 0.0, c, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    m = rotate * m;
    
    // 3. 平移（到當前位置）
    mat4 translate = mat4(1.0);
    translate[3] = vec4(currentPos, 1.0);
    m = translate * m;
    
    return m;
}

void main() {
    // 設定殘影數量
    int layers = 5;
    float timeLag = 0.15; // 每一層殘影的時間延遲

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