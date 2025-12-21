#ifndef CINEMATIC_DIRECTOR_H
#define CINEMATIC_DIRECTOR_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include "camera.h"

// 關鍵影格結構
struct Keyframe {
    float time;                    // 時間點（秒）
    glm::vec3 cameraPosition;     // 相機位置
    glm::vec3 cameraTarget;        // 相機目標（看向的位置）
    float transitionDuration;      // 過渡時間（用於平滑插值）
    
    Keyframe(float t, glm::vec3 pos, glm::vec3 target, float duration = 1.0f)
        : time(t), cameraPosition(pos), cameraTarget(target), transitionDuration(duration) {}
};

// 鏡頭類型枚舉
enum class ShotType {
    WIDE_SHOT,      // 遠景
    MEDIUM_SHOT,    // 中景
    CLOSE_UP,       // 特寫
    FOLLOW,         // 跟隨
    ORBIT           // 環繞
};

// 前置宣告
class AnimatedModel;

class CinematicDirector {
public:
    // 建構子：接收相機、角色模型和動畫模型的引用
    CinematicDirector(camera_t& cam, glm::mat4& charModel, glm::mat4& cartModel, AnimatedModel* animModel);
    
    // 開始播放動畫
    void Start();
    
    // 停止播放
    void Stop();
    
    // 重置到開始
    void Reset();
    
    // 每一幀更新（傳入 deltaTime）
    void Update(float deltaTime);
    
    // 是否正在播放
    bool IsPlaying() const { return m_IsPlaying; }
    
    // 獲取當前時間
    float GetCurrentTime() const { return m_GlobalTime; }
    
    // 設置是否循環播放
    void SetLoop(bool loop) { m_Loop = loop; }
    
    // 跳轉到指定時間
    void SeekTo(float time);
    
    // 獲取角色模型矩陣
    glm::mat4 GetCharacterModelMatrix() const { return m_CharacterModel; }
    
    // 獲取車子模型矩陣
    glm::mat4 GetCartModelMatrix() const { return m_CartModel; }
    
    // 更新角色移動（公開方法，供外部調用）
    void UpdateCharacterMovement(float currentTime);
    
    // 更新車子移動（公開方法，供外部調用）
    void UpdateCartMovement(float currentTime);
    
    // 更新頭部旋轉（公開方法，供外部調用）
    void UpdateHeadRotation(float currentTime);

private:
    // 參考到主程式的相機和模型
    camera_t& m_Camera;
    glm::mat4& m_CharacterModel;
    glm::mat4& m_CartModel;
    AnimatedModel* m_AnimatedModel;  // 用於控制頭部旋轉
    
    // 時間控制
    float m_GlobalTime;
    bool m_IsPlaying;
    bool m_Loop;
    
    // 關鍵影格序列
    std::vector<Keyframe> m_Keyframes;
    
    // 內部函式：初始化關鍵影格序列
    void InitializeKeyframes();
    
    // 內部函式：執行不同類型的鏡頭
    void PlayWideShot(float t);
    void PlayMediumShot(float t);
    void PlayCloseUp(float t);
    void PlayFollowShot(float t);
    void PlayOrbitShot(float t);
    
    // 內部函式：關鍵影格插值
    void InterpolateBetweenKeyframes(float currentTime);
    
    // 內部函式：更新相機
    void UpdateCamera(glm::vec3 position, glm::vec3 target);
    
    
    // 內部函式：平滑插值（Ease in-out）
    float SmoothStep(float t);
};

#endif

