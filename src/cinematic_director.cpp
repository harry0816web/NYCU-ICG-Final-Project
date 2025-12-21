#include "header/cinematic_director.h"
#include "header/animated_model.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <glm/gtc/matrix_transform.hpp>

CinematicDirector::CinematicDirector(camera_t& cam, glm::mat4& charModel, glm::mat4& cartModel, AnimatedModel* animModel)
    : m_Camera(cam)
    , m_CharacterModel(charModel)
    , m_CartModel(cartModel)
    , m_AnimatedModel(animModel)
    , m_GlobalTime(0.0f)
    , m_IsPlaying(false)
    , m_Loop(false)
    , m_RollStartTime(-1.0f) // 初始化為-1，表示尚未開始翻滾
    , m_RollFinished(false)  // 初始化為false，表示翻滾尚未結束
    , m_CollisionTime(-1.0f) // 初始化為-1，表示尚未發生碰撞
    , m_CollisionPosition(0.0f, 0.0f, 500.0f) // 初始化碰撞位置
{
    InitializeKeyframes();
    // 初始化時設置初始位置（確保與 main_animated.cpp 中的初始位置一致）
    UpdateCharacterMovement(0.0f);
    UpdateCartMovement(0.0f);
    std::cout << "CinematicDirector: Constructor called, " << m_Keyframes.size() << " keyframes initialized" << std::endl;
}

void CinematicDirector::Start() {
    m_GlobalTime = 0.0f;
    m_IsPlaying = true;
    std::cout << "Cinematic Director: Started (Time: " << m_GlobalTime << "s)" << std::endl;
    
    // 立即更新到第一個關鍵影格
    if (m_Keyframes.size() > 0) {
        InterpolateBetweenKeyframes(0.0f);
        std::cout << "  Camera position set to first keyframe" << std::endl;
    } else {
        std::cout << "  WARNING: No keyframes available!" << std::endl;
    }
}

void CinematicDirector::Stop() {
    m_IsPlaying = false;
    std::cout << "Cinematic Director: Stopped" << std::endl;
}

void CinematicDirector::Reset() {
    m_GlobalTime = 0.0f;
    m_IsPlaying = false;
    if (m_Keyframes.size() > 0) {
        UpdateCamera(m_Keyframes[0].cameraPosition, m_Keyframes[0].cameraTarget);
    }
}

void CinematicDirector::Update(float deltaTime) {
    if (!m_IsPlaying) return;
    
    m_GlobalTime += deltaTime;
    
    // 如果循環播放，重置時間
    if (m_Loop && m_Keyframes.size() > 0) {
        float totalDuration = m_Keyframes.back().time + m_Keyframes.back().transitionDuration;
        if (m_GlobalTime >= totalDuration) {
            m_GlobalTime = fmod(m_GlobalTime, totalDuration);
        }
    }
    
    // 使用關鍵影格插值
    InterpolateBetweenKeyframes(m_GlobalTime);
}

// 使用指定時間更新相機（用於與動畫時間同步）
void CinematicDirector::UpdateCameraWithTime(float currentTime) {
    if (!m_IsPlaying) return;
    
    // 更新全局時間（用於其他更新函數）
    m_GlobalTime = currentTime;
    
    // 先更新角色移動，確保模型矩陣是最新的
    UpdateCharacterMovement(currentTime);
    
    // 控制車子移動：第5秒開始往人的方向移動
    UpdateCartMovement(currentTime);
    
    // 控制頭部旋轉：在第5秒開始讓頭部轉向鏡頭
    if (m_AnimatedModel) {
        UpdateHeadRotation(currentTime);
    }
    
    // 使用關鍵影格插值更新相機（在角色移動更新之後）
    InterpolateBetweenKeyframes(currentTime);
    
    // 每秒輸出相機位置和lookat，用於記錄關鍵幀（使用整數秒，確保時間一致）
    // 所有位置都是世界座標
    static float lastCameraOutputTime = -1.0f;
    if (currentTime - lastCameraOutputTime >= 1.0f || lastCameraOutputTime < 0.0f) {
        int timeInt = (int)currentTime;
        std::cout << "Camera Keyframe: Time=" << timeInt 
                  << "s, Position (World)=(" << std::fixed << std::setprecision(4) << m_Camera.position.x << ", " 
                  << m_Camera.position.y << ", " << m_Camera.position.z 
                  << "), LookAt (World)=(" << m_Camera.target.x << ", " << m_Camera.target.y << ", " 
                  << m_Camera.target.z << ")" << std::endl;
        lastCameraOutputTime = currentTime;
    }
}

void CinematicDirector::SeekTo(float time) {
    m_GlobalTime = time;
    InterpolateBetweenKeyframes(m_GlobalTime);
}

void CinematicDirector::InitializeKeyframes() {
    m_Keyframes.clear();
    
    // 鏡頭腳本：左後方側拍 → 特寫臉部
    // 角色從(150, 0, 500)走到(0, 0, 500)
    
    // 0-3秒：鏡頭固定在指定位置
    // 使用用戶提供的關鍵幀位置
    m_Keyframes.push_back(Keyframe(0.0f, 
        glm::vec3(31.3615f, 21.9684f, 120.577f),  // 相機位置：用戶提供的關鍵幀
        glm::vec3(0.0f, 0.0f, 0.0f),   // LookAt: 幾乎是(0,0,0)
        3.0f));                             // 持續3秒
    
    // 3-5秒：過渡階段，從第一個位置平滑移動到第二個位置
    // 相機從第一個位置移動到特寫位置
    m_Keyframes.push_back(Keyframe(3.0f,
        glm::vec3(31.3615f, 21.9684f, 120.577f),  // 起始位置：第一個關鍵幀（與0-3秒結束位置相同）
        glm::vec3(0.0f, 0.0f, 0.0f),    // LookAt: (0,0,0)
        2.0f));                             // 持續2秒，過渡到特寫位置
    
    // 5-7秒：臉部特寫，固定在此位置
    // 相機在特寫位置，x座標往-x一點，跟到人物的x座標（0）
    m_Keyframes.push_back(Keyframe(5.0f,
        glm::vec3(-5.0f, 10.3934f, 70.7176f),    // 相機位置：x往-x一點（-5），跟到人物x座標（0），其他保持不變
        glm::vec3(0.0f, 0.0f, 0.0f),    // LookAt: (0,0,0)
        2.0f));                             // 持續2秒（5-7秒）
    
    // 7-8秒：相機位置不變，但lookat的z座標反過來，開始拍車子
    // 相機保持在特寫位置，但lookat指向車子方向（z座標反過來）
    m_Keyframes.push_back(Keyframe(7.0f,
        glm::vec3(-5.0f, 10.3934f, 70.7176f),    // 相機位置：保持不變
        glm::vec3(0.0f, 0.0f, 150.0f),    // LookAt: z座標反過來（從0變成150，指向車子初始位置）
        1.0f));                             // 持續1秒（7-8秒）
    
    // 9-15秒：固定位置和LookAt(0,0,0)
    m_Keyframes.push_back(Keyframe(9.0f,
        glm::vec3(100.0f, 50.0f, 0.0f),    // 相機位置：固定在(100, 50, 0)
        glm::vec3(0.0f, 0.0f, 0.0f),    // LookAt: 固定在(0,0,0)
        6.0f));                             // 持續到15秒（9-15秒）
    
    // 15秒：開始跟隨人物，相機位置從(100, 50, 0)開始
    m_Keyframes.push_back(Keyframe(15.0f,
        glm::vec3(100.0f, 50.0f, 0.0f),    // 相機位置：從(100, 50, 0)開始
        glm::vec3(0.0f, 0.0f, 0.0f),    // LookAt: 將在插值邏輯中動態跟隨人物（這裡只是占位）
        1.0f));                             // 持續到16秒（15-16秒）
    
    // 16秒：相機位置固定在(0, 50, 0)，LookAt跟隨人物
    m_Keyframes.push_back(Keyframe(16.0f,
        glm::vec3(0.0f, 50.0f, 0.0f),    // 相機位置：固定在(0, 50, 0)
        glm::vec3(0.0f, 0.0f, 0.0f),    // LookAt: 將在插值邏輯中動態跟隨人物（這裡只是占位）
        5.0f));                             // 持續到21秒（16-21秒）
    
    // 21秒：結束關鍵幀，相機位置保持在(0, 50, 0)，LookAt跟隨人物
    m_Keyframes.push_back(Keyframe(21.0f,
        glm::vec3(0.0f, 50.0f, 0.0f),    // 相機位置：保持在(0, 50, 0)
        glm::vec3(0.0f, 0.0f, 0.0f),    // LookAt: 將在插值邏輯中動態跟隨人物（這裡只是占位）
        0.0f));                             // 結束
    
    std::cout << "Cinematic Director: Initialized " << m_Keyframes.size() << " keyframes" << std::endl;
    std::cout << "  Keyframe 1 (0-3s): Fixed camera position" << std::endl;
    std::cout << "    Camera: (31.3615, 21.9684, 120.577) -> Target: (0, 0, 0)" << std::endl;
    std::cout << "  Keyframe 2 (3-5s): Transition to close-up" << std::endl;
    std::cout << "    Camera: (31.3615, 21.9684, 120.577) -> (-5, 10.3934, 70.7176), Target: (0, 0, 0)" << std::endl;
    std::cout << "  Keyframe 3 (5-7s): Face close-up, fixed position" << std::endl;
    std::cout << "    Camera: (-5, 10.3934, 70.7176) -> Target: (0, 0, 0)" << std::endl;
    std::cout << "  Keyframe 4 (7-8s): Camera fixed, lookat z reversed, following car" << std::endl;
    std::cout << "    Camera: (-5, 10.3934, 70.7176) -> Target: (following car, z reversed)" << std::endl;
    std::cout << "  Keyframe 5 (9-15s): Fixed camera position" << std::endl;
    std::cout << "    Camera: (100, 50, 0) -> Target: (0, 0, 0)" << std::endl;
    std::cout << "  Keyframe 6 (15-16s): Camera transitions to (0, 50, 0)" << std::endl;
    std::cout << "    Camera: (100, 50, 0) -> (0, 50, 0), Target: (follows character)" << std::endl;
    std::cout << "  Keyframe 7 (16-21s): Camera fixed at (0, 50, 0), Target: (follows character)" << std::endl;
    std::cout << "    Camera: (0, 50, 0) -> Target: (follows character)" << std::endl;
    std::cout << "  Note: Head rotation starts at 5 seconds, Car starts moving at 6 seconds (6-15s)" << std::endl;
}

void CinematicDirector::InterpolateBetweenKeyframes(float currentTime) {
    if (m_Keyframes.empty()) return;
    
    // 找到當前時間所在的關鍵影格區間
    size_t currentIndex = 0;
    for (size_t i = 0; i < m_Keyframes.size() - 1; i++) {
        if (currentTime >= m_Keyframes[i].time && currentTime < m_Keyframes[i + 1].time) {
            currentIndex = i;
            break;
        }
        if (i == m_Keyframes.size() - 2) {
            currentIndex = i;
        }
    }
    
    // 從實際模型矩陣中提取角色和車子位置（用於相機跟隨）
    // 角色模型矩陣構建順序：scale(0.1) * translate(pos) * rotate
    // 在glm中，矩陣乘法是從右到左的，所以實際執行順序是：先rotate，再translate，再scale
    // 最終矩陣 = S * T * R，其中：
    //   S = scale(0.1) = [0.1, 0, 0, 0; 0, 0.1, 0, 0; 0, 0, 0.1, 0; 0, 0, 0, 1]
    //   T = translate(pos) = [1, 0, 0, pos.x; 0, 1, 0, pos.y; 0, 0, 1, pos.z; 0, 0, 0, 1]
    //   R = rotate(...)，第四列是 [0, 0, 0, 1]（旋轉不影響平移部分）
    // 所以 m_CharacterModel[3] = S * T * R * [0,0,0,1] = S * T * [0,0,0,1] = [pos.x*0.1, pos.y*0.1, pos.z*0.1, 1]
    // 因此，世界座標位置 = m_CharacterModel[3].xyz / 0.1
    glm::vec3 characterPos = glm::vec3(m_CharacterModel[3]) / 0.1f;
    
    // 車子模型矩陣：translate(pos) * rotate * scale(10)
    // 所以 m_CartModel[3] 包含 pos，不需要除以 10
    glm::vec3 cartPos = glm::vec3(m_CartModel[3]);
    
    // 如果超過最後一個關鍵影格（21秒後），固定在第16秒的位置和LookAt追蹤人物
    if (currentTime >= m_Keyframes.back().time) {
        // 重新提取人物位置（確保是最新的世界座標）
        glm::vec3 actualCharacterPos = glm::vec3(m_CharacterModel[3]) / 0.1f;
        // 固定在(0, 50, 0)的位置，LookAt追蹤人物 - 所有位置都是世界座標
        glm::vec3 cameraPos = glm::vec3(0.0f, 50.0f, 0.0f); // 第16秒的位置 - 世界座標
        // LookAt追蹤人物位置，Y座標根據時間調整：
        // 20秒前：Y座標固定為50（水平看向人物）
        // 20-21秒：Y座標從50平滑過渡到-150
        // 21秒後：Y座標固定為-150（更向下看向人物，確保能看到地面上的人物）
        float lookAtY;
        if (currentTime < 20.0f) {
            lookAtY = 50.0f; // 20秒前固定為50
        } else if (currentTime < 21.0f) {
            // 20-21秒：從50平滑過渡到-150
            float transitionProgress = (currentTime - 20.0f) / 1.0f; // 0.0 到 1.0
            transitionProgress = glm::clamp(transitionProgress, 0.0f, 1.0f);
            float smoothProgress = SmoothStep(transitionProgress); // 使用平滑函數
            lookAtY = glm::mix(50.0f, -200.0f, smoothProgress);
        } else {
            lookAtY = -200.0f; // 21秒後固定為-200
        }
        glm::vec3 cameraTarget = glm::vec3(actualCharacterPos.x, lookAtY, actualCharacterPos.z);
        UpdateCamera(cameraPos, cameraTarget);
        return;
    }
    
    // 獲取當前和前一個關鍵影格
    const Keyframe& keyframe1 = m_Keyframes[currentIndex];
    const Keyframe& keyframe2 = m_Keyframes[currentIndex + 1];
    
    // 計算插值因子（0.0 到 1.0）
    float timeInSegment = currentTime - keyframe1.time;
    float segmentDuration = keyframe2.time - keyframe1.time;
    float t = (segmentDuration > 0.0f) ? (timeInSegment / segmentDuration) : 0.0f;
    t = glm::clamp(t, 0.0f, 1.0f);
    
    // 使用平滑插值
    float smoothT = SmoothStep(t);
    
    // 插值相機位置和目標（使用關鍵影格插值作為基礎）
    glm::vec3 cameraPos = glm::mix(keyframe1.cameraPosition, keyframe2.cameraPosition, smoothT);
    glm::vec3 cameraTarget = glm::mix(keyframe1.cameraTarget, keyframe2.cameraTarget, smoothT);
    
    // 16-21秒：相機位置固定在(0, 50, 0)，LookAt追蹤人物
    if (currentTime >= 16.0f && currentTime <= 21.0f) {
        // 重新提取人物位置（確保是最新的世界座標）
        // 模型矩陣構建順序：scale(0.1) * translate(pos) * rotate
        // 世界座標位置 = m_CharacterModel[3].xyz / 0.1
        glm::vec3 actualCharacterPos = glm::vec3(m_CharacterModel[3]) / 0.1f;
        
        cameraPos = glm::vec3(0.0f, 50.0f, 0.0f); // 固定在(0, 50, 0) - 世界座標
        // LookAt追蹤人物位置，Y座標根據時間調整：
        // 20秒前：Y座標固定為50（水平看向人物）
        // 20-21秒：Y座標從50平滑過渡到-150
        // 21秒後：Y座標固定為-150（更向下看向人物，確保能看到地面上的人物）
        float lookAtY;
        if (currentTime < 20.0f) {
            lookAtY = 50.0f; // 20秒前固定為50
        } else if (currentTime < 21.0f) {
            // 20-21秒：從50平滑過渡到-150
            float transitionProgress = (currentTime - 20.0f) / 1.0f; // 0.0 到 1.0
            transitionProgress = glm::clamp(transitionProgress, 0.0f, 1.0f);
            float smoothProgress = SmoothStep(transitionProgress); // 使用平滑函數
            lookAtY = glm::mix(50.0f, -150.0f, smoothProgress);
        } else {
            lookAtY = -150.0f; // 21秒後固定為-150
        }
        cameraTarget = glm::vec3(actualCharacterPos.x, lookAtY, actualCharacterPos.z);
        
        // 調試輸出：檢查人物位置和模型矩陣（所有位置都是世界座標）
        static float lastDebugTime = -1.0f;
        if (currentTime - lastDebugTime >= 1.0f || lastDebugTime < 0.0f) {
            glm::vec3 rawModelPos = glm::vec3(m_CharacterModel[3]);
            std::cout << "DEBUG 16-21s (World Coordinates): " << std::endl;
            std::cout << "  Raw Model[3]=(" << rawModelPos.x << ", " << rawModelPos.y << ", " << rawModelPos.z << ")" << std::endl;
            std::cout << "  Character World Pos=(" << actualCharacterPos.x << ", " 
                      << actualCharacterPos.y << ", " << actualCharacterPos.z << ")" << std::endl;
            std::cout << "  Camera World Pos=(" << cameraPos.x << ", " 
                      << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
            std::string lookAtYDesc;
            if (currentTime < 20.0f) {
                lookAtYDesc = "50";
            } else if (currentTime < 21.0f) {
                lookAtYDesc = "50->-150 (transitioning)";
            } else {
                lookAtYDesc = "-150";
            }
            std::cout << "  LookAt World Pos (Y = " << lookAtYDesc << ")=(" << cameraTarget.x << ", " 
                      << cameraTarget.y << ", " << cameraTarget.z << ")" << std::endl;
            lastDebugTime = currentTime;
        }
    }
    // 15-16秒：相機位置從(100, 50, 0)過渡到(0, 50, 0)，LookAt追蹤人物
    else if (currentTime >= 15.0f && currentTime < 16.0f) {
        // 重新提取人物位置（確保是最新的世界座標）
        glm::vec3 actualCharacterPos = glm::vec3(m_CharacterModel[3]) / 0.1f;
        // 相機位置使用插值（從(100, 50, 0)到(0, 50, 0)）- 世界座標
        // LookAt追蹤人物位置，Y座標根據時間調整：
        // 20秒前：Y座標固定為50（水平看向人物）
        // 20-21秒：Y座標從50平滑過渡到-150
        // 21秒後：Y座標固定為-150（更向下看向人物，確保能看到地面上的人物）
        float lookAtY;
        if (currentTime < 20.0f) {
            lookAtY = 50.0f; // 20秒前固定為50
        } else if (currentTime < 21.0f) {
            // 20-21秒：從50平滑過渡到-150
            float transitionProgress = (currentTime - 20.0f) / 1.0f; // 0.0 到 1.0
            transitionProgress = glm::clamp(transitionProgress, 0.0f, 1.0f);
            float smoothProgress = SmoothStep(transitionProgress); // 使用平滑函數
            lookAtY = glm::mix(50.0f, -150.0f, smoothProgress);
        } else {
            lookAtY = -150.0f; // 21秒後固定為-150
        }
        cameraTarget = glm::vec3(actualCharacterPos.x, lookAtY, actualCharacterPos.z);
    }
    // 9-15秒：固定在第9秒的位置和LookAt(0,0,0)，不追蹤人物
    else if (currentTime >= 9.0f && currentTime < 15.0f) {
        // 固定在第9秒的相機位置和LookAt
        cameraPos = glm::vec3(100.0f, 50.0f, 0.0f); // 第9秒的位置
        cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); // LookAt固定在(0,0,0)
    }
    // 5-7秒：使用關鍵幀插值，x座標跟隨人物的x座標，其他固定
    else if (currentTime >= 5.0f && currentTime < 7.0f) {
        // 相機x座標跟隨人物的x座標（往-x一點）
        cameraPos.x = characterPos.x - 5.0f; // 往-x一點，跟到人物的x座標
        // 其他座標保持關鍵幀插值的結果
    }
    // 7-9秒：保持原樣（相機位置不變，但lookat跟隨車子移動）
    else if (currentTime >= 7.0f && currentTime < 9.0f) {
        // 相機位置保持不變（使用關鍵幀插值）
        // lookat跟隨車子移動（指向車子當前位置）
        cameraTarget = glm::vec3(cartPos.x, cartPos.y, cartPos.z); // 直接看向車子位置
    }
    
    // 更新相機
    UpdateCamera(cameraPos, cameraTarget);
}

void CinematicDirector::UpdateCamera(glm::vec3 position, glm::vec3 target) {
    m_Camera.position = position;
    m_Camera.target = target;
    
    // 確保 worldUp 已設置（如果沒有，使用默認值）
    if (glm::length(m_Camera.worldUp) < 0.1f) {
        m_Camera.worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    }
    
    // 計算相機方向
    m_Camera.front = glm::normalize(target - position);
    m_Camera.right = glm::normalize(glm::cross(m_Camera.front, m_Camera.worldUp));
    m_Camera.up = glm::normalize(glm::cross(m_Camera.right, m_Camera.front));
    
    // 更新 yaw 和 pitch（如果需要）
    glm::vec3 direction = glm::normalize(target - position);
    m_Camera.pitch = glm::degrees(asin(direction.y));
    m_Camera.yaw = glm::degrees(atan2(direction.z, direction.x));
    
    // 更新 radius（用於兼容現有的相機系統）
    m_Camera.radius = glm::length(target - position);
}

float CinematicDirector::SmoothStep(float t) {
    // Ease in-out 平滑函數
    return t * t * (3.0f - 2.0f * t);
}

// 以下函式可以根據需要實現特定類型的鏡頭
void CinematicDirector::PlayWideShot(float t) {
    // 遠景實現
    glm::vec3 pos = glm::vec3(50.0f, 30.0f, 50.0f);
    glm::vec3 target = glm::vec3(0.0f, 0.0f, -25.0f);
    UpdateCamera(pos, target);
}

void CinematicDirector::PlayMediumShot(float t) {
    // 中景實現
    glm::vec3 pos = glm::vec3(0.0f, 5.0f, 20.0f);
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
    UpdateCamera(pos, target);
}

void CinematicDirector::PlayCloseUp(float t) {
    // 特寫實現
    glm::vec3 pos = glm::vec3(5.0f, 2.0f, 0.0f);
    glm::vec3 target = glm::vec3(0.0f, 1.0f, 0.0f);
    UpdateCamera(pos, target);
}

void CinematicDirector::PlayFollowShot(float t) {
    // 跟隨鏡頭：相機跟隨角色移動
    // 可以從角色模型矩陣中提取位置
    glm::vec3 charPos = glm::vec3(m_CharacterModel[3]);
    glm::vec3 offset = glm::vec3(0.0f, 5.0f, 10.0f);
    UpdateCamera(charPos + offset, charPos);
}

void CinematicDirector::PlayOrbitShot(float t) {
    // 環繞鏡頭：相機圍繞角色旋轉
    float angle = t * 0.5f; // 旋轉速度
    float radius = 20.0f;
    glm::vec3 charPos = glm::vec3(m_CharacterModel[3]);
    glm::vec3 pos = charPos + glm::vec3(
        cos(angle) * radius,
        10.0f,
        sin(angle) * radius
    );
    UpdateCamera(pos, charPos);
}

void CinematicDirector::UpdateHeadRotation(float currentTime) {
    if (!m_AnimatedModel) return;
    
    // 在第5秒開始讓頭部轉向鏡頭，持續1秒完成（5.0-6.0秒）
    float headRotationStartTime = 5.0f;
    float headRotationEndTime = 6.0f;
    float headRotationDuration = 1.0f; // 1秒完成轉頭（5-6秒）
    
    if (currentTime < headRotationStartTime) {
        // 5秒前：清除頭部和上半身旋轉（嘗試所有可能的骨骼名稱）
        // 清除頭部旋轉
        m_AnimatedModel->clearBoneAdditionalRotation("mixamorig:Head");
        m_AnimatedModel->clearBoneAdditionalRotation("Head");
        m_AnimatedModel->clearBoneAdditionalRotation("head");
        m_AnimatedModel->clearBoneAdditionalRotation("mixamorig:Neck");
        m_AnimatedModel->clearBoneAdditionalRotation("Neck");
        m_AnimatedModel->clearBoneAdditionalRotation("neck");
        m_AnimatedModel->clearBoneAdditionalRotation("mixamorig:Neck1");
        m_AnimatedModel->clearBoneAdditionalRotation("Neck1");
        m_AnimatedModel->clearBoneAdditionalRotation("neck1");
        
        // 清除上半身旋轉
        m_AnimatedModel->clearBoneAdditionalRotation("mixamorig:Spine2");
        m_AnimatedModel->clearBoneAdditionalRotation("Spine2");
        m_AnimatedModel->clearBoneAdditionalRotation("spine2");
        m_AnimatedModel->clearBoneAdditionalRotation("mixamorig:Spine1");
        m_AnimatedModel->clearBoneAdditionalRotation("Spine1");
        m_AnimatedModel->clearBoneAdditionalRotation("spine1");
        m_AnimatedModel->clearBoneAdditionalRotation("mixamorig:Spine");
        m_AnimatedModel->clearBoneAdditionalRotation("Spine");
        m_AnimatedModel->clearBoneAdditionalRotation("spine");
        m_AnimatedModel->clearBoneAdditionalRotation("mixamorig:UpperChest");
        m_AnimatedModel->clearBoneAdditionalRotation("UpperChest");
        m_AnimatedModel->clearBoneAdditionalRotation("upperChest");
        return;
    }
    
    // 計算轉頭進度（0.0 到 1.0）
    // 確保在 5.0 秒時 progress = 0，在 6.0 秒時 progress = 1.0
    float rotationProgress = (currentTime - headRotationStartTime) / headRotationDuration;
    
    // 在 5.0-6.0 秒之間：progress 從 0.0 到 1.0
    // 在 6.0 秒後：progress 保持為 1.0（完成狀態）
    if (currentTime >= headRotationEndTime) {
        rotationProgress = 1.0f; // 6秒後保持完成狀態，不要清除
    } else {
        rotationProgress = glm::clamp(rotationProgress, 0.0f, 1.0f);
    }
    
    // 在5-6秒期間，使用線性插值讓轉頭更快更明顯（不使用平滑函數）
    // 6秒後，保持完成狀態（smoothProgress = 1.0）
    // 其他時間使用平滑函數讓轉頭更自然
    float smoothProgress;
    if (currentTime >= headRotationStartTime && currentTime <= headRotationEndTime) {
        // 直接使用線性插值，讓轉頭更快
        smoothProgress = rotationProgress;
    } else if (currentTime > headRotationEndTime) {
        // 6秒後，保持完成狀態，不變
        smoothProgress = 1.0f;
    } else {
        // 5秒前，使用平滑函數
        smoothProgress = SmoothStep(rotationProgress);
    }
    
    // 計算頭部應該轉向的角度（繞X軸，上下轉，點頭/抬頭）
    // 根據相機位置計算頭部應該轉向的垂直角度
    
    float maxRotationAngle = 70.0f; // 最大旋轉角度（度），增大到90度讓轉動更明顯
    
    // 根據相機位置計算頭部應該轉向的角度
    // 獲取角色當前位置（從模型矩陣中提取）
    glm::vec3 characterPos = glm::vec3(m_CharacterModel[3]); // 獲取平移部分
    // 頭部位置：角色位置 + 頭部相對位置（考慮縮放0.1）
    glm::vec3 headPosition = characterPos + glm::vec3(0.0f, 1.5f, 0.0f); // 頭部在角色上方約1.5單位（縮放後）
    glm::vec3 toCamera = m_Camera.position - headPosition;
    glm::vec3 cameraDirection = glm::normalize(toCamera);
    
    // 計算頭部應該轉向的角度（繞X軸，上下轉）
    // 計算從頭部到相機的垂直角度（pitch角）
    // 使用 atan2 計算垂直角度：atan2(y, length(x,z))
    float horizontalDist = sqrt(cameraDirection.x * cameraDirection.x + cameraDirection.z * cameraDirection.z);
    float targetPitch = glm::degrees(atan2(-cameraDirection.y, horizontalDist));
    
    // 在5-6秒期間，強制使用最大旋轉角度，讓轉頭動作更明顯
    // 相機在下方時，點頭（正角度）
    // 相機在上方時，抬頭（負角度）
    // 6秒後，保持最後的角度不變
    static float finalTargetPitch = 0.0f; // 保存6秒時的最終角度
    static bool finalTargetPitchSet = false; // 標記是否已設置最終角度
    
    // 在5秒前，重置標記（為動畫重新開始做準備）
    if (currentTime < headRotationStartTime) {
        finalTargetPitchSet = false;
    }
    
    if (currentTime >= headRotationStartTime && currentTime <= headRotationEndTime) {
        // 在轉頭期間，根據相機Y位置決定方向，使用最大角度
        if (m_Camera.position.y < headPosition.y) {
            // 相機在頭部下方，向下點頭
            targetPitch = maxRotationAngle;
        } else {
            // 相機在頭部上方，向上抬頭
            targetPitch = -maxRotationAngle;
        }
        // 保存最終角度（在接近6秒時）
        if (currentTime >= headRotationEndTime - 0.01f && !finalTargetPitchSet) {
            finalTargetPitch = targetPitch;
            finalTargetPitchSet = true;
        }
    } else if (currentTime > headRotationEndTime) {
        // 6秒後，使用保存的最終角度，保持不變
        if (finalTargetPitchSet) {
            targetPitch = finalTargetPitch;
        } else {
            // 如果還沒有設置，使用當前計算的角度（備用方案）
            targetPitch = (m_Camera.position.y < headPosition.y) ? maxRotationAngle : -maxRotationAngle;
            finalTargetPitch = targetPitch;
            finalTargetPitchSet = true;
        }
    } else {
        // 5秒前，如果計算出的角度太小，使用固定的大角度
        if (abs(targetPitch) < 15.0f) {
            // 根據相機Y位置決定方向
            if (m_Camera.position.y < headPosition.y) {
                // 相機在頭部下方，向下點頭
                targetPitch = maxRotationAngle;
            } else {
                // 相機在頭部上方，向上抬頭
                targetPitch = -maxRotationAngle;
            }
        }
        // 限制旋轉角度範圍（避免過度轉頭）
        targetPitch = glm::clamp(targetPitch, -maxRotationAngle, maxRotationAngle);
    }
    
    // 根據進度插值旋轉角度（從0度到目標角度）
    // 在5-6秒期間，使用線性插值讓轉頭更快更明顯（不使用平滑函數）
    // 6秒後，保持最終角度不變（smoothProgress = 1.0）
    float rotationAngle = -1 * smoothProgress * targetPitch;
    
    // 添加調試輸出（5-6秒期間）
    static float lastDebugTime = -1.0f;
    if (currentTime >= headRotationStartTime && currentTime <= headRotationEndTime) {
        if (currentTime - lastDebugTime > 0.2f) {
            // std::cout << "Head rotation: time=" << currentTime << "s, progress=" << rotationProgress 
            //           << ", smoothProgress=" << smoothProgress << ", targetPitch=" << targetPitch 
            //           << ", rotationAngle=" << rotationAngle << " degrees" << std::endl;
            lastDebugTime = currentTime;
        }
    }
    
    // 創建旋轉四元數（繞X軸旋轉，上下轉）
    // X軸向右，正角度向下轉（點頭），負角度向上轉（抬頭）
    glm::quat headRotation = glm::angleAxis(glm::radians(rotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));
    
    // 嘗試多個可能的骨骼名稱（Mixamo 可能有不同的命名）
    // 注意：有些模型使用小寫名稱（如 "head", "neck"）
    std::vector<std::string> possibleHeadBones = {
        "mixamorig:Head",
        "Head",
        "head",              // 小寫版本
        "mixamorig:Neck",
        "Neck",
        "neck",              // 小寫版本
        "mixamorig:Neck1",
        "Neck1",
        "neck1"              // 小寫版本
    };
    
    // 上半身骨骼名稱（用於跟隨頭部轉動）
    std::vector<std::string> possibleSpineBones = {
        "mixamorig:Spine2",
        "Spine2",
        "spine2",
        "mixamorig:Spine1",
        "Spine1",
        "spine1",
        "mixamorig:Spine",
        "Spine",
        "spine",
        "mixamorig:UpperChest",
        "UpperChest",
        "upperChest"
    };
    
    // 應用旋轉到頭部骨骼
    bool headRotationApplied = false;
    for (const auto& boneName : possibleHeadBones) {
        // 檢查骨骼是否存在於骨骼映射中
        if (m_AnimatedModel->m_BoneInfoMap.find(boneName) != m_AnimatedModel->m_BoneInfoMap.end()) {
            m_AnimatedModel->setBoneAdditionalRotation(boneName, headRotation);
            headRotationApplied = true;
            // 在5-6秒期間輸出（每0.2秒一次）
            static float lastHeadPrintTime = -1.0f;
            if (currentTime >= headRotationStartTime && currentTime <= headRotationEndTime) {
                if (currentTime - lastHeadPrintTime > 0.2f) {
                    std::cout << "Head rotation applied to bone: " << boneName << " (angle: " << rotationAngle << " degrees, progress: " << smoothProgress << ")" << std::endl;
                    lastHeadPrintTime = currentTime;
                }
            }
            break;
        }
    }
    
    // 如果沒有找到頭部骨骼，輸出警告
    if (!headRotationApplied && currentTime >= headRotationStartTime && currentTime <= headRotationEndTime) {
        static bool warned = false;
        if (!warned) {
            std::cout << "WARNING: Could not find head bone! Tried: ";
            for (const auto& name : possibleHeadBones) {
                std::cout << name << " ";
            }
            std::cout << std::endl;
            warned = true;
        }
    }
    
    // 應用相同的旋轉到上半身骨骼（讓上半身跟隨頭部轉動）
    // 使用相同的角度（100%）
    float spineRotationFactor = 1.0f; // 上半身轉動比例（100%的頭部角度，相同角度）
    float spineRotationAngle = rotationAngle * spineRotationFactor;
    glm::quat spineRotation = glm::angleAxis(glm::radians(spineRotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));
    
    bool spineRotationApplied = false;
    for (const auto& boneName : possibleSpineBones) {
        // 檢查骨骼是否存在於骨骼映射中
        if (m_AnimatedModel->m_BoneInfoMap.find(boneName) != m_AnimatedModel->m_BoneInfoMap.end()) {
            m_AnimatedModel->setBoneAdditionalRotation(boneName, spineRotation);
            spineRotationApplied = true;
            if (currentTime - headRotationStartTime < 0.1f) {
                // 只在開始時輸出一次
                std::cout << "Spine rotation applied to bone: " << boneName << " (angle: " << spineRotationAngle << " degrees)" << std::endl;
            }
            // 只應用第一個找到的脊柱骨骼
            break;
        }
    }
    
    // 檢查是否有應用旋轉
    bool rotationApplied = headRotationApplied || spineRotationApplied;
    
    if (!rotationApplied && currentTime - headRotationStartTime < 0.5f) {
        // 只在開始時輸出一次警告
        std::cout << "WARNING: Could not find head/neck bone. Tried: ";
        for (const auto& name : possibleHeadBones) {
            std::cout << name << " ";
        }
        std::cout << std::endl;
        std::cout << "Available bones: ";
        for (const auto& pair : m_AnimatedModel->m_BoneInfoMap) {
            if (pair.first.find("Head") != std::string::npos || 
                pair.first.find("Neck") != std::string::npos ||
                pair.first.find("head") != std::string::npos ||
                pair.first.find("neck") != std::string::npos) {
                std::cout << pair.first << " ";
            }
        }
        std::cout << std::endl;
    }
}

void CinematicDirector::UpdateCharacterMovement(float currentTime) {
    // 0-5秒：從初始位置 x=150 走到 x=0（保持原來的朝向和Z位置）
    float walkStartTime = 0.0f;
    float walkDuration = 5.0f;
    
    // 碰撞檢測：基於車子和人的距離，而不是固定時間
    // 飛行從碰撞開始，一直持續到第20秒才落地
    float flyEndTime = 20.0f; // 飛行結束時間（固定在第20秒）
    float rollDuration = 5.0f; // 翻滾持續5秒
    
    // 保持原來的朝向（-90度，朝向+X方向，右邊）
    float rotationAngle = -90.0f; // 朝向+X方向（右邊），保持原來的方向
    
    // 起始位置：使用初始位置（x=150, z=500）
    glm::vec3 startPos = glm::vec3(150.0f, 0.0f, 500.0f);
    // 目標位置：x=0, 保持z=500
    glm::vec3 walkEndPos = glm::vec3(0.0f, 0.0f, 500.0f);
    
    // 飛行目標位置：往 -z +y 方向飛出去，飛得更遠
    // 從碰撞點往 -z 方向移動，Z軸移動量是原來的三倍
    float flyDistanceZ = 2400.0f; // Z方向飛行距離（往 -z，從800增加到2400，變成三倍）
    // flyEndPos 將在飛行邏輯中根據實際碰撞位置計算
    
    if (currentTime < walkStartTime) {
        // 0秒前：保持在初始位置
        static bool printed = false;
        if (!printed) {
            std::cout << "Character initial position: x=" << startPos.x << ", y=" << startPos.y << ", z=" << startPos.z << std::endl;
            printed = true;
        }
        m_CharacterModel = glm::mat4(1.0f);
        m_CharacterModel = glm::scale(m_CharacterModel, glm::vec3(0.1f, 0.1f, 0.1f));
        m_CharacterModel = glm::translate(m_CharacterModel, startPos);
        m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        return;
    }
    
    // 檢查碰撞：5秒後開始檢測（角色已到達目標位置）
    // 從模型矩陣中提取實際的世界位置（考慮縮放）
    // 角色模型矩陣構建順序：scale(0.1) * translate(pos) * rotate
    //   - 先translate，再scale，所以矩陣[3]被縮放了，需要除以0.1
    // 車子模型矩陣構建順序：translate(pos) * rotate * scale(10)
    //   - translate在最前面，scale在最後，所以矩陣[3]不受縮放影響，不需要除以10
    
    // 獲取角色實際世界位置（從模型矩陣的平移部分提取，除以縮放因子0.1）
    glm::vec3 characterPos = glm::vec3(m_CharacterModel[3]) / 0.1f;
    
    // 獲取車子實際世界位置（從模型矩陣的平移部分提取，translate在scale之前，所以不需要除以10）
    glm::vec3 cartPos = glm::vec3(m_CartModel[3]);
    
    // 每秒輸出人物和車子的世界座標（使用整數秒，與Camera Keyframe格式一致）
    static float lastPositionPrintTime = -1.0f;
    if (currentTime - lastPositionPrintTime >= 1.0f || lastPositionPrintTime < 0.0f) {
        int timeInt = (int)currentTime;
        // 輸出世界座標位置
        std::cout << "Time=" << timeInt << "s: Character World Position=(" 
                  << std::fixed << std::setprecision(4) << characterPos.x << ", " 
                  << characterPos.y << ", " << characterPos.z 
                  << "), Cart World Position=(" << cartPos.x << ", " << cartPos.y << ", " << cartPos.z << ")" << std::endl;
        lastPositionPrintTime = currentTime;
    }
    
    // 計算距離（簡單的2D距離，忽略Y軸）
    float dx = cartPos.x - characterPos.x;
    float dz = cartPos.z - characterPos.z;
    float distance = sqrt(dx * dx + dz * dz);
    
    // 碰撞檢測：要求X軸距離很小，且總距離大於等於閾值（450）
    float xDistance = abs(dx);
    float zDistance = abs(dz);
    
    // 碰撞條件：X軸距離很小（< 10），且總距離大於等於450
    // 當距離大於等於450時觸發碰撞（避免破圖，提前觸發）
    float collisionDistance = 410.0f;
    bool collisionByX = (xDistance < 10.0f) && (distance >= collisionDistance);
    bool collisionByDistance = (distance >= collisionDistance);
    bool distanceCondition = collisionByX || collisionByDistance;
    
    // 車子開始移動的時間（與UpdateCartMovement中的cartMoveStartTime一致）
    float cartMoveStartTime = 6.0f;
    
    // 檢查是否發生碰撞（需要同時滿足：角色已到達目標位置、車子已開始移動、距離條件滿足、尚未發生碰撞）
    bool characterReady = (currentTime >= walkStartTime + walkDuration); // 5秒後角色到達目標位置
    bool cartStarted = (currentTime >= cartMoveStartTime); // 6秒後車子開始移動
    bool notCollidedYet = (m_CollisionTime < 0.0f);
    bool collisionDetected = characterReady && cartStarted && notCollidedYet && distanceCondition;
    
    // 關閉碰撞檢測的調試輸出
    // if (characterReady && cartStarted && notCollidedYet && !distanceCondition) {
    //     static float lastDebugTime = -1.0f;
    //     if (currentTime - lastDebugTime > 1.0f) {
    //         std::cout << "Collision NOT detected: dx=" << xDistance << ", dz=" << zDistance 
    //                   << ", distance=" << distance << ", collisionByX=" << collisionByX 
    //                   << ", collisionByDistance=" << collisionByDistance << std::endl;
    //         lastDebugTime = currentTime;
    //     }
    // }
    
    // 關閉碰撞檢測的詳細調試輸出
    // static float lastCollisionCheckTime = -1.0f;
    // if (currentTime >= cartMoveStartTime && collisionTime < 0.0f) {
    //     if (currentTime - lastCollisionCheckTime > 0.2f) {
    //         float dx = cartPos.x - characterPos.x;
    //         float dz = cartPos.z - characterPos.z;
    //         std::cout << "Collision check: time=" << currentTime << "s" << std::endl;
    //         std::cout << "  char_pos=(" << characterPos.x << ", " << characterPos.y << ", " << characterPos.z << ")" << std::endl;
    //         std::cout << "  cart_pos=(" << cartPos.x << ", " << cartPos.y << ", " << cartPos.z << ")" << std::endl;
    //         std::cout << "  dx=" << dx << ", dz=" << dz << ", distance=" << distance << std::endl;
    //         std::cout << "  collisionByX=" << collisionByX << ", collisionByDistance=" << collisionByDistance << std::endl;
    //         glm::vec3 charModelPos = glm::vec3(m_CharacterModel[3]);
    //         glm::vec3 cartModelPos = glm::vec3(m_CartModel[3]);
    //         std::cout << "  char_model[3]=(" << charModelPos.x << ", " << charModelPos.y << ", " << charModelPos.z << ")" << std::endl;
    //         std::cout << "  cart_model[3]=(" << cartModelPos.x << ", " << cartModelPos.y << ", " << cartModelPos.z << ")" << std::endl;
    //         lastCollisionCheckTime = currentTime;
    //     }
    // }
    
    // 如果發生碰撞，記錄碰撞時間和位置
    if (collisionDetected) {
        m_CollisionTime = currentTime;
        m_CollisionPosition = characterPos; // 記錄碰撞時的位置
        // 計算翻滾開始時間（飛行結束時，固定在第20秒）
        m_RollStartTime = flyEndTime; // 翻滾開始時間固定在第20秒
        // 計算飛行持續時間（從碰撞時間到第20秒）
        float flyDuration = flyEndTime - m_CollisionTime;
        // 輸出碰撞檢測信息（用於調試）
        // 輸出世界座標位置
        std::cout << "COLLISION DETECTED! Distance: " << distance << ", Time: " << currentTime << "s, Character World Position: (" 
                  << m_CollisionPosition.x << ", " << m_CollisionPosition.y << ", " << m_CollisionPosition.z << "), Cart World Position: ("
                  << cartPos.x << ", " << cartPos.y << ", " << cartPos.z << "), Fly Duration: " << flyDuration 
                  << "s, Roll Start Time: " << m_RollStartTime << "s" << std::endl;
    }
    
    // 如果已發生碰撞，開始飛行
    if (m_CollisionTime >= 0.0f && currentTime >= m_CollisionTime) {
        // 計算飛行持續時間（從碰撞時間到第20秒）
        float flyDuration = flyEndTime - m_CollisionTime;
        
        // 計算飛行目標位置（從碰撞點往 -z 方向移動）- 在if塊外定義，讓翻滾邏輯也能使用
        glm::vec3 flyEndPos = glm::vec3(m_CollisionPosition.x, m_CollisionPosition.y, m_CollisionPosition.z - flyDistanceZ);
        
        if (currentTime < flyEndTime) {
        // 被撞後飛行（往 -z +y 方向）
        float flyProgress = (currentTime - m_CollisionTime) / flyDuration;
        flyProgress = glm::clamp(flyProgress, 0.0f, 1.0f);
        
        // 使用平滑函數（開始快，結束慢，模擬重力）
        float smoothFlyProgress = SmoothStep(flyProgress);
        
        // 計算飛行位置（拋物線運動：y 先上升後下降）
        glm::vec3 currentPos;
        float t = smoothFlyProgress;
        // Y軸：拋物線（先上升後下降）- Y增長量是原來的三倍
        float maxHeight = 1080.0f; // 最大高度（從360增加到1080，讓Y增長量是原來的三倍）
        float yPos = 4.0f * maxHeight * t * (1.0f - t); // 拋物線：y = 4h * t * (1-t)
        // X和Z：線性移動（往 -z 方向）
        currentPos.x = glm::mix(m_CollisionPosition.x, flyEndPos.x, t);
        currentPos.y = m_CollisionPosition.y + yPos; // 加上拋物線高度
        currentPos.z = glm::mix(m_CollisionPosition.z, flyEndPos.z, t);
        
        // 確保 Y 軸有明顯的上升（+y 方向）
        // 在飛行前半段，Y 應該明顯上升
        if (t < 0.5f) {
            // 前半段：快速上升
            float riseFactor = 1.0f + (0.5f - t) * 2.0f; // 開始時更快上升（增強因子增加到2.0）
            yPos *= riseFactor;
            currentPos.y = m_CollisionPosition.y + yPos;
        }
        
        // 計算身體朝向：頭部朝向世界座標 -z（屏幕裡），身體（臉部）朝向世界座標 +y（向上）
        // 
        // 初始狀態：角色面向 +X 方向（右邊），身體向上（+Y），頭部在 +X 方向（向前），臉部也在 +X 方向
        // 目標狀態：頭部朝向 (0, 0, -1)（屏幕裡），身體（臉部）朝向 (0, 1, 0)（向上）
        //
        // 分析：
        // - 初始：頭部在模型空間是 +X 方向 (1,0,0)，臉部也在 +X 方向 (1,0,0)
        // - 要讓頭部朝向 (0,0,-1)：需要先繞X軸旋轉-90度，再繞Y軸旋轉-90度（上一個版本是對的）
        // - 要讓臉部朝向 (0,1,0)：臉部在模型空間是+X，需要讓它在世界空間朝向+Y
        //   如果先繞X軸旋轉-90度，再繞Y軸旋轉-90度：
        //   - 繞X軸-90度：(1,0,0) -> (1,0,0) (X軸旋轉不影響X方向)
        //   - 繞Y軸-90度：(1,0,0) -> (0,0,-1) (頭部朝向-Z)
        //   這樣臉部會朝向-Z，不是+Y
        //
        // 重新分析：要讓臉部朝向+Y，頭部朝向-Z
        // - 如果臉部在模型空間是+X，要讓它在世界空間朝向+Y：
        //   - 繞Z軸旋轉-90度：(1,0,0) -> (0,1,0) ✓
        // - 但這樣頭部也會變成+Y，不是-Z
        //
        // 正確理解：臉部朝向(0,1,0)可能指的是臉部正面朝向+Y，而不是頭部方向
        // 如果角色初始面向+X，臉部正面是+X方向
        // 要讓臉部正面朝向+Y，需要：
        // - 先繞Z軸旋轉-90度：臉部從+X轉到+Y ✓
        // - 但頭部方向也會變成+Y，不是-Z
        //
        // 重新思考：可能需要先繞X軸旋轉，再繞Z軸旋轉，或者調整角度
        // 如果先繞X軸旋轉-90度，再繞Z軸旋轉-90度：
        // - 繞X軸-90度：(1,0,0) -> (1,0,0)
        // - 繞Z軸-90度：(1,0,0) -> (0,1,0) ✓ 臉部朝向+Y
        // 但頭部方向也會是+Y，不是-Z
        //
        // 實際上，如果頭部朝向-Z是對的，那麼臉部應該也朝向-Z
        // 但用戶說要讓臉部朝向+Y，這可能意味著需要額外的旋轉
        //
        // 嘗試：先繞X軸旋轉-90度，再繞Y軸旋轉-90度（頭部朝向-Z），然後再繞X軸旋轉90度讓臉部朝向+Y？
        // 這樣會讓頭部又變回原來的方向
        //
        // 正確方案：先繞X軸旋轉-90度，再繞Z軸旋轉-90度
        // - 繞X軸-90度：(1,0,0) -> (1,0,0)
        // - 繞Z軸-90度：(1,0,0) -> (0,1,0) ✓ 臉部朝向+Y
        // 但這樣頭部也會朝向+Y，不是-Z
        //
        // 重新理解：用戶說"上一個的頭部朝向是對的"，上一個是先X後Y，頭部朝向-Z
        // 現在要讓"身體（臉部）朝向(0,1,0)"
        // 可能臉部指的是身體的正面方向，而不是頭部方向
        // 如果角色初始面向+X，身體正面是+X方向
        // 要讓身體正面朝向+Y，同時頭部朝向-Z：
        // - 先繞X軸旋轉-90度，再繞Y軸旋轉-90度：頭部朝向-Z ✓
        // - 但身體正面（臉部）會朝向-Z，不是+Y
        //
        // 可能需要：先繞X軸旋轉-90度，再繞Y軸旋轉-90度，然後再繞X軸旋轉90度？
        // 這樣頭部會變回原來的方向
        //
        // 正確理解：臉部朝向(0,1,0)可能指的是臉部向上看，而不是向前看
        // 如果角色初始面向+X，臉部向前+X
        // 要讓臉部向上+Y，同時頭部朝向-Z：
        // 這需要複雜的旋轉組合
        //
        // 正確的旋轉：先繞X軸旋轉-90度，再繞Y軸旋轉-90度
        // - 繞X軸-90度：(1,0,0) -> (1,0,0) (X軸旋轉不影響X方向)
        // - 繞Y軸-90度：(1,0,0) -> (0,0,-1) 頭部朝向-Z ✓
        float bodyRotationX = -90.0f; // 繞X軸旋轉-90度
        float bodyRotationY = -90.0f; // 繞Y軸旋轉-90度，讓頭部從+X轉到-Z
        
        // 在飛行過程中完成Y軸旋轉-90度（從0度到-90度）
        float additionalYRotation = smoothFlyProgress * -90.0f; // 在飛行過程中完成Y軸旋轉-90度
        
        // 更新角色模型矩陣
        m_CharacterModel = glm::mat4(1.0f);
        m_CharacterModel = glm::scale(m_CharacterModel, glm::vec3(0.1f, 0.1f, 0.1f));
        m_CharacterModel = glm::translate(m_CharacterModel, currentPos);
        // 先繞Y軸旋轉（在世界座標系中，因為先應用），在飛行過程中完成Y軸旋轉-90度
        m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(additionalYRotation), glm::vec3(0.0f, 1.0f, 0.0f));
        // 然後應用飛行pose：先繞X軸旋轉，再繞Y軸旋轉（讓頭部朝向-Z）
        m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(bodyRotationX), glm::vec3(1.0f, 0.0f, 0.0f));
        m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(bodyRotationY), glm::vec3(0.0f, 1.0f, 0.0f));
        
        // 計算頭部朝向向量（在模型空間中，頭部是 +Z 方向（向前），需要轉換到世界空間）
        // 頭部在模型空間中是 (0, 0, 1)，需要乘以旋轉矩陣
        glm::mat4 rotationMatrix = glm::mat4(1.0f);
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(additionalYRotation), glm::vec3(0.0f, 1.0f, 0.0f));
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(bodyRotationX), glm::vec3(1.0f, 0.0f, 0.0f));
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(bodyRotationY), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec4 headDirectionModel = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f); // 模型空間中頭部方向是 +Z（向前）
        glm::vec4 headDirectionWorld = rotationMatrix * headDirectionModel;
        glm::vec3 headDirection = glm::normalize(glm::vec3(headDirectionWorld));
        
        // 計算身體朝向向量（在模型空間中，身體正面是 +X 方向（向右），需要轉換到世界空間）
        // 身體正面在模型空間中是 (1, 0, 0)，需要乘以旋轉矩陣
        glm::vec4 bodyDirectionModel = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f); // 模型空間中身體正面方向是 +X（向右）
        glm::vec4 bodyDirectionWorld = rotationMatrix * bodyDirectionModel;
        glm::vec3 bodyDirection = glm::normalize(glm::vec3(bodyDirectionWorld));
        
        // 關閉角色方向的調試輸出
        // static float lastFlyPrintTime = -1.0f;
        // if (currentTime - lastFlyPrintTime > 0.5f) {
        //     std::cout << "Character flying: pos=(" << currentPos.x << ", " << currentPos.y << ", " << currentPos.z 
        //               << "), head_dir(world)=(" << headDirection.x << ", " << headDirection.y << ", " << headDirection.z 
        //               << "), body_dir(world)=(" << bodyDirection.x << ", " << bodyDirection.y << ", " << bodyDirection.z 
        //               << "), time=" << currentTime << "s, progress=" << flyProgress << std::endl;
        //     lastFlyPrintTime = currentTime;
        // }
        return;
        } else {
            // 飛行結束後：落地並翻滾（第20秒開始）
            // Y軸旋轉-90度已經在飛行過程中完成
            float rollStartTime = flyEndTime; // 翻滾開始時間（固定在第20秒，飛行結束時，Y軸旋轉已完成）
            float rollEndTime = rollStartTime + rollDuration; // 翻滾結束時間
            
            // 飛行結束時的旋轉（保持飛行pose）
            float flyEndRotationX = -90.0f; // 繞X軸旋轉-90度
            float flyEndRotationY = -90.0f; // 繞Y軸旋轉-90度
            float additionalYRotation = -90.0f; // Y軸旋轉-90度（已在飛行過程中完成）
            
            if (currentTime < rollEndTime) {
                // 翻滾階段：直接繞世界座標X軸翻滾
                float rollProgress = (currentTime - rollStartTime) / rollDuration;
                rollProgress = glm::clamp(rollProgress, 0.0f, 1.0f);
                
                // 使用平滑函數讓翻滾更自然
                float smoothRollProgress = SmoothStep(rollProgress);
                
                // 翻滾三圈少90度 = 3 * 360度 - 90度 = 990度（負角度，反方向）
                float totalRollAngle = -990.0f;
                float currentRollAngle = smoothRollProgress * totalRollAngle;
                
                // 更新角色模型矩陣
                m_CharacterModel = glm::mat4(1.0f);
                m_CharacterModel = glm::scale(m_CharacterModel, glm::vec3(0.1f, 0.1f, 0.1f));
                m_CharacterModel = glm::translate(m_CharacterModel, flyEndPos);
                
                // 要繞世界座標X軸翻滾，需要先繞X軸旋轉，再繞Y軸旋轉
                // 1. 先繞世界座標X軸翻滾（在世界座標系中，因為先應用）
                float rollRotationX = currentRollAngle; // 繞X軸翻滾（負角度，反方向）
                m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(rollRotationX), glm::vec3(1.0f, 0.0f, 0.0f));
                
                // 2. 然後應用Y軸旋轉-90度（已在飛行過程中完成）
                m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(additionalYRotation), glm::vec3(0.0f, 1.0f, 0.0f));
                
                // 3. 然後保持飛行結束時的pose
                m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(flyEndRotationX), glm::vec3(1.0f, 0.0f, 0.0f));
                m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(flyEndRotationY), glm::vec3(0.0f, 1.0f, 0.0f));
                
                // 計算頭頂朝向（在模型空間中，頭頂是 +Y 方向，需要轉換到世界空間）
                // 旋轉順序：先X軸翻滾，再Y軸轉-90度，再飛行pose
                glm::mat4 rotationMatrix = glm::mat4(1.0f);
                rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rollRotationX), glm::vec3(1.0f, 0.0f, 0.0f));
                rotationMatrix = glm::rotate(rotationMatrix, glm::radians(additionalYRotation), glm::vec3(0.0f, 1.0f, 0.0f));
                rotationMatrix = glm::rotate(rotationMatrix, glm::radians(flyEndRotationX), glm::vec3(1.0f, 0.0f, 0.0f));
                rotationMatrix = glm::rotate(rotationMatrix, glm::radians(flyEndRotationY), glm::vec3(0.0f, 1.0f, 0.0f));
                glm::vec4 topDirectionModel = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f); // 模型空間中頭頂方向是 +Y（向上）
                glm::vec4 topDirectionWorld = rotationMatrix * topDirectionModel;
                glm::vec3 topDirection = glm::normalize(glm::vec3(topDirectionWorld));
                
                // 關閉頭頂朝向的調試輸出
                // static float lastRollPrintTime = -1.0f;
                // if (currentTime - lastRollPrintTime > 0.2f) {
                //     std::cout << "Character rolling: top_dir(world)=(" << topDirection.x << ", " << topDirection.y << ", " << topDirection.z 
                //               << "), roll_angle=" << currentRollAngle << "deg, time=" << currentTime << "s" << std::endl;
                //     lastRollPrintTime = currentTime;
                // }
                
                return;
            } else {
                // 翻滾結束後：停在最終位置
                if (!m_RollFinished) {
                    m_RollFinished = true;
                    std::cout << "Roll finished! Triggering explode effect..." << std::endl;
                }
                m_CharacterModel = glm::mat4(1.0f);
                m_CharacterModel = glm::scale(m_CharacterModel, glm::vec3(0.1f, 0.1f, 0.1f));
                m_CharacterModel = glm::translate(m_CharacterModel, flyEndPos);
                
                // 要繞世界座標X軸翻滾，需要先繞X軸旋轉，再繞Y軸旋轉
                // 1. 先繞世界座標X軸翻滾三圈少90度（在世界座標系中，因為先應用）
                float rollRotationX = -990.0f; // 繞X軸翻滾三圈少90度（-990度，負角度，反方向）
                m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(rollRotationX), glm::vec3(1.0f, 0.0f, 0.0f));
                
                // 2. 然後應用Y軸旋轉-90度（已在飛行過程中完成）
                m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(additionalYRotation), glm::vec3(0.0f, 1.0f, 0.0f));
                
                // 3. 然後保持飛行結束時的pose
                m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(flyEndRotationX), glm::vec3(1.0f, 0.0f, 0.0f));
                m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(flyEndRotationY), glm::vec3(0.0f, 1.0f, 0.0f));
                
                // 計算頭頂朝向
                glm::mat4 rotationMatrix = glm::mat4(1.0f);
                rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rollRotationX), glm::vec3(1.0f, 0.0f, 0.0f));
                rotationMatrix = glm::rotate(rotationMatrix, glm::radians(additionalYRotation), glm::vec3(0.0f, 1.0f, 0.0f));
                rotationMatrix = glm::rotate(rotationMatrix, glm::radians(flyEndRotationX), glm::vec3(1.0f, 0.0f, 0.0f));
                rotationMatrix = glm::rotate(rotationMatrix, glm::radians(flyEndRotationY), glm::vec3(0.0f, 1.0f, 0.0f));
                // 關閉翻滾結束後的頭頂朝向輸出
                // glm::vec4 topDirectionModel = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
                // glm::vec4 topDirectionWorld = rotationMatrix * topDirectionModel;
                // glm::vec3 topDirection = glm::normalize(glm::vec3(topDirectionWorld));
                // std::cout << "Character after rolling: top_dir(world)=(" << topDirection.x << ", " << topDirection.y << ", " << topDirection.z << ")" << std::endl;
                
        return;
            }
        }
    }
    
    // 0-5秒：正常走路
    if (currentTime > walkStartTime + walkDuration) {
        // 5秒後：停在 x=0 位置（等待被撞）
        m_CharacterModel = glm::mat4(1.0f);
        m_CharacterModel = glm::scale(m_CharacterModel, glm::vec3(0.1f, 0.1f, 0.1f));
        m_CharacterModel = glm::translate(m_CharacterModel, walkEndPos);
        m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        return;
    }
    
    // 計算移動進度（0.0 到 1.0）
    float walkProgress = (currentTime - walkStartTime) / walkDuration;
    walkProgress = glm::clamp(walkProgress, 0.0f, 1.0f);
    
    // 使用平滑函數讓移動更自然
    float smoothProgress = SmoothStep(walkProgress);
    
    // 插值計算當前位置
    glm::vec3 currentPos = glm::mix(startPos, walkEndPos, smoothProgress);
    
    // 關閉走路階段的位置輸出（已由每秒輸出取代）
    // static float lastPrintTime = -1.0f;
    // if (currentTime - lastPrintTime > 0.5f) {
    //     std::cout << "Character position: x=" << currentPos.x << ", y=" << currentPos.y << ", z=" << currentPos.z << " (time=" << currentTime << "s)" << std::endl;
    //     lastPrintTime = currentTime;
    // }
    
    // 更新角色模型矩陣
    m_CharacterModel = glm::mat4(1.0f);
    m_CharacterModel = glm::scale(m_CharacterModel, glm::vec3(0.1f, 0.1f, 0.1f));
    m_CharacterModel = glm::translate(m_CharacterModel, currentPos);
    m_CharacterModel = glm::rotate(m_CharacterModel, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // 保持原來的朝向
}

void CinematicDirector::UpdateCartMovement(float currentTime) {
    // 6-15秒：車子往-Z方向開（延長3秒，讓速度變慢，可以明顯看到motion blur）
    float cartMoveStartTime = 6.0f;
    float cartMoveDuration = 9.0f; // 9秒完成移動（從6秒增加到9秒，讓速度變慢）
    
    // 起始位置：使用初始位置（與 main_animated.cpp 中的初始位置一致）
    glm::vec3 startPos = glm::vec3(0.0f, 0.0f, 150.0f);
    // 目標位置：往-Z方向移動，多走100單位（從70改為-30）
    glm::vec3 endPos = glm::vec3(0.0f, 0.0f, 70.0f);
    
    if (currentTime < cartMoveStartTime) {
        // 6秒前：車子保持在初始位置（與 main_animated.cpp 中的初始位置一致）
        m_CartModel = glm::mat4(1.0f);
        m_CartModel = glm::translate(m_CartModel, startPos); // 使用初始位置
        m_CartModel = glm::rotate(m_CartModel, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // 車頭朝向螢幕裡面（-Z方向）
        m_CartModel = glm::scale(m_CartModel, glm::vec3(10.0f, 10.0f, 10.0f));
        return;
    }
    
    if (currentTime > cartMoveStartTime + cartMoveDuration) {
        // 15秒後：車子到達-Z方向的位置
        m_CartModel = glm::mat4(1.0f);
        m_CartModel = glm::translate(m_CartModel, endPos);
        m_CartModel = glm::rotate(m_CartModel, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // 車頭朝向螢幕裡面（-Z方向）
        m_CartModel = glm::scale(m_CartModel, glm::vec3(10.0f, 10.0f, 10.0f));
        return;
    }
    
    // 計算移動進度（0.0 到 1.0）
    float moveProgress = (currentTime - cartMoveStartTime) / cartMoveDuration;
    moveProgress = glm::clamp(moveProgress, 0.0f, 1.0f);
    
    // 使用平滑函數讓移動更自然
    float smoothProgress = SmoothStep(moveProgress);
    
    // 插值計算當前位置
    glm::vec3 currentPos = glm::mix(startPos, endPos, smoothProgress);
    
    // 更新車子模型矩陣
    m_CartModel = glm::mat4(1.0f);
    m_CartModel = glm::translate(m_CartModel, currentPos);
    m_CartModel = glm::rotate(m_CartModel, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // 車頭朝向螢幕裡面（-Z方向）
    m_CartModel = glm::scale(m_CartModel, glm::vec3(10.0f, 10.0f, 10.0f));
}

